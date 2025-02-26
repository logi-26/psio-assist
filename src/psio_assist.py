#!/usr/bin/env python3
#
#  psio-assist
#
#  This is basically a GUI and wrapper for the binmerge and cue2cu2 scripts
#  This has a very basic UI for selecting a directory that contains bin/cue files
#  The script will check if the bin files need to be merged, create the cu2 file and add the game cover
#
#  Copyright (C) 2021 LoGi26
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# System imports
from sys import argv, exit
from os.path import exists, join, dirname, basename, splitext, abspath
from os import walk, listdir, scandir, mkdir, remove, rename
from shutil import copyfile, move, rmtree
from time import sleep
from pathlib2 import Path
from json import load, dumps
from tkinter import *
from tkinter import filedialog
import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from ttkbootstrap import Progressbar
from ttkbootstrap.dialogs import MessageDialog
from ttkbootstrap import Treeview
from ttkbootstrap import Style
from ttkbootstrap import Scrollbar
from ttkbootstrap import Labelframe
from ttkbootstrap import Label
from ttkbootstrap import Button
from ttkbootstrap import Floodgauge
from ttkbootstrap import Checkbutton

# Local imports
from game_files import Game, Cuesheet, Binfile
from binmerge import set_binmerge_error_log_path, start_bin_merge, read_cue_file
from cue2cu2 import set_cu2_error_log_path, start_cue2cu2
from db import ensure_database_exists, select, extract_game_cover_blob

REGION_CODES = ['DTLS_', 'SCES_', 'SLES_', 'SLED_', 'SCED_', 'SCUS_', 'SLUS_', 'SLPS_', 'SCAJ_', 'SLKA_', 'SLPM_', 'SCPS_', 'SCPM_', 'PCPX_', 'PAPX_', 'PTPX_', 'LSP0_', 'LSP1_', 'LSP2_', 'LSP9_', 'SIPS_', 'ESPM_', 'SCZS_', 'SPUS_', 'PBPX_', 'LSP_']
CURRENT_REVISION = 0.8
PROGRESS_STATUS = 'Status:'
MAX_GAME_NAME_LENGTH = 56

# Global variables
game_list = []
covers_path = None

# *****************************************************************************************************************
def _get_stored_theme():
    try:
        with open(CONFIG_FILE_PATH) as config_file:
            data = load(config_file)
            return data['theme']
    except:
        return 'darkly'  # default theme if config file doesn't exist
# *****************************************************************************************************************

# Create the main window
window = ttk.Window(title=f'PSIO Game Assistant v{CURRENT_REVISION}', themename=_get_stored_theme(), size=[800,710])
style = Style()

# Configure style for progress bars to match button height
style.configure(
    "primary.Horizontal.TProgressbar",
    thickness=28  # Ajustado para corresponder à altura dos botões
)

style.configure(
    "primary.Horizontal.TFloodgauge",
    thickness=28  # Ajustado para corresponder à altura dos botões
)

# Create Tkinter variables after window initialization
src_path = StringVar(value='')
merge_bin_files = BooleanVar(value=False)
force_cu2 = BooleanVar(value=False)
validate_game_name = BooleanVar(value=False)
auto_rename = BooleanVar(value=False)
add_cover_art = BooleanVar(value=False)
create_multi_disc = BooleanVar(value=False)

# Configure grid weight for main window
window.grid_columnconfigure(0, weight=1)
window.grid_rowconfigure(0, weight=0)  # Browse frame
window.grid_rowconfigure(1, weight=1)  # Game list frame
window.grid_rowconfigure(2, weight=0)  # Tools frame
window.grid_rowconfigure(3, weight=0)  # Progress frame

# Get the directory paths based on the scripts location
script_root_dir = Path(abspath(dirname(argv[0])))
covers_path = join(dirname(script_root_dir), 'covers')
error_log_file = join(dirname(script_root_dir), 'errors.txt')

CONFIG_FILE_PATH = join(script_root_dir, 'config')

# Set the error log path for all of the scripts
set_cu2_error_log_path(error_log_file)
set_binmerge_error_log_path(error_log_file)


# *****************************************************************************************************************
# Function that processes the games
def _process_games():
    total_games = len(game_list)
    covers_not_found = 0
    for index, game in enumerate(game_list):
        game_id = game.id
        if not game_id:
            print(f"WARNING: Game ID is None for {game.cue_sheet.game_name}. Skipping this game.")
            continue

        game_name = game.cue_sheet.game_name

        game_full_path = join(game.directory_path, game.directory_name)
        cue_full_path = join(game_full_path, game.cue_sheet.file_name)

        label_progress.configure(text=f'{PROGRESS_STATUS} In Progress - {game_name}')

        print(f'GAME_ID: {game_id}')
        print(f'GAME_NAME: {game_name}')
        print(f'GAME_PATH: {game_full_path}')
        print(f'CUE_PATH: {cue_full_path}')

        if merge_bin_files.get() and len(game.cue_sheet.bin_files) > 1:
            print('MERGING BIN FILES...')
            label_progress.configure(text=f'{PROGRESS_STATUS} In Progress - Merging bin files - {game_name}')
            _merge_bin_files(game, game_name, game_full_path, cue_full_path)
            start_cue2cu2(cue_full_path, f'{game_name}.bin')

        if force_cu2.get() and not game.cu2_present:
            print('GENERATING CU2...')
            label_progress.configure(text=f'{PROGRESS_STATUS} In Progress - Generating cu2 file - {game_name}')
            start_cue2cu2(cue_full_path, f'{game_name}.bin')

        if auto_rename.get():
            print('RENAMING THE GAME FILES...')
            label_progress.configure(text=f'{PROGRESS_STATUS} In Progress - Renaming - {game_name}')
            try:
                redump_game_name = _game_name_validator(_get_redump_name(game_id))
                if redump_game_name:
                    _rename_game(game_full_path, game_name, redump_game_name)
            except Exception as e:
                print(f"WARNING: Could not rename game {game_name}. Skipping this game. Error: {e}")

        if validate_game_name.get() and not auto_rename.get():
            if len(game_name) > MAX_GAME_NAME_LENGTH or '.' in game_name:
                print('VALIDATING THE GAME NAME...')
                label_progress.configure(text=f'{PROGRESS_STATUS} In Progress - Validating name - {game_name}')
                new_game_name = _game_name_validator(game_name)
                print(f'new_game_name: {new_game_name}')
                if new_game_name != game_name:
                    _rename_game(game_full_path, game_name, new_game_name)

        if add_cover_art.get() and not game.cover_art_present:
            try:
                print('ADDING THE GAME COVER ART...')
                label_progress.configure(text=f'{PROGRESS_STATUS} In Progress - Adding cover art - {game_name}')
                _copy_game_cover(game_full_path, game_id, game_name)
            except Exception as e:
                print(f"WARNING: Could not add cover art for {game_name}. Skipping this game. Error: {e}")
                covers_not_found += 1

        # Update the progress bar
        progress = int((index + 1) / total_games * 100)
        _update_progress_bar(progress)

    label_progress.configure(text=f'{PROGRESS_STATUS} Done')


# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to merge multi-bin files
def _merge_bin_files(game, game_name, game_full_path, cue_full_path):
    # Create a temp directory to store the merged bin file
    temp_game_dir = join(game_full_path, 'temp_dir')
    if not exists(temp_game_dir):
        try:
            mkdir(temp_game_dir)
        except OSError as error:
            pass
    if exists(temp_game_dir):
        label_progress.configure(text=f'{PROGRESS_STATUS} Merging bin files')
        start_bin_merge(cue_full_path, game_name, temp_game_dir)

        # If the bin files have been merged and the new cue file has been generated
        temp_bin_path = join(temp_game_dir, f'{game_name}.bin')
        temp_cue_path = join(temp_game_dir, f'{game_name}.cue')
        if exists(temp_bin_path) and exists(temp_cue_path):
            # Delete the original cue_sheet and bin files
            remove(cue_full_path)
            for original_bin_file in game.cue_sheet.bin_files:
                remove(join(game_full_path, original_bin_file.file_name))

            # Move the newly merged bin_file and cue_sheet back into the game directory
            move(temp_bin_path, join(game_full_path, f'{game_name}.bin'))
            move(temp_cue_path, join(game_full_path, f'{game_name}.cue'))

        rmtree(temp_game_dir)
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to rename a game and all associated files
def _rename_game(game_full_path, game_name, new_game_name):
	original_bin_file = join(game_full_path, f'{game_name}.bin')
	original_cue_file = join(game_full_path, f'{game_name}.cue')
	original_cu2_file = join(game_full_path, f'{game_name}.cu2')

	# Rename bin file
	if exists(original_bin_file):
		rename(original_bin_file, join(game_full_path, f'{new_game_name}.bin'))

	# Rename cue file and edit the cue file contents to match
	if exists(original_cue_file):
		# Edit cue file content
		cue_path = Path(original_cue_file)
		cue_text = cue_path.read_text()
		cue_text = cue_text.replace(game_name, new_game_name)
		cue_path.write_text(cue_text)
		rename(original_cue_file, join(game_full_path, f'{new_game_name}.cue'))

	# Rename cu2 file
	if exists(original_cu2_file):
		rename(original_cu2_file, join(game_full_path, f'{new_game_name}.cu2'))

	# Rename bmp file
	if exists(join(game_full_path, f'{game_name}.bmp')):
		rename(join(game_full_path, f'{game_name}.bmp'), join(game_full_path, f'{new_game_name}.bmp'))
	if exists(join(game_full_path, f'{game_name}.BMP')):
		rename(join(game_full_path, f'{game_name}.BMP'), join(game_full_path, f'{new_game_name}.BMP'))
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to validate the game name (ensure it is not too long and does not contain periods)
def _game_name_validator(game_name):
    if '.' in game_name:
        game_name = game_name.replace('.', '_')

    if len(game_name) > MAX_GAME_NAME_LENGTH:
        game_name = game_name[:MAX_GAME_NAME_LENGTH]

    return game_name
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to check if the game is a multi-disc game
def _is_multi_disc(game):
	return int(game.disc_number) > 0 if game.disc_number is not None else None
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to check if the game is a multi-bin game
def _is_multi_bin(game):
	return len(game.cue_sheet.bin_files) > 1
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to check if all of the required bin files exist
def _all_game_files_exist(game):
	for bin_file in game.cue_sheet.bin_files:
		if not exists(bin_file.file_path):
			return False
	return True
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function that generates a MULTIDISC.LST file for multi-disc games
def _generate_multidisc_file(game_dir):
	bin_files = [f for f in listdir(join(output_path, game_dir)) if f.endswith('.bin')]

	# If there is more than 1 bin file, this should be a multi-disc game
	multi_disc_bins = []
	if len(bin_files) > 1:
		for bin_file in bin_files:
			disc_number = _get_disc_number(_get_game_id(join(output_path, game_dir, bin_file)))
			if disc_number > 0:
				multi_disc_bins.insert(disc_number-1, bin_file)

	# Create the MULTIDISC.LST file
	if len(multi_disc_bins) > 0:
		with open(join(output_path, game_dir, 'MULTIDISC.LST'), 'w') as multi_disc_file:
			for count, binfile in enumerate(multi_disc_bins):
				if count < len(multi_disc_bins)-1:
					#multi_disc_file.write(f'{binfile}\n')
					multi_disc_file.write(f'{binfile}\r')
				else:
					multi_disc_file.write(binfile)
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to get the game name (using names from redump and the psx data-centre)
def _get_redump_name(game_id):
    response = select(f'''SELECT name, disc_number FROM games WHERE game_id = "{game_id.replace('-','_')}";''')
    if response is not None and response != []:
        game_name, disc_number = response[0]

        # Ensure that the game name (including disc number and extension) is not more than 60 chars
        if validate_game_name.get():
            if int(disc_number) > 0:
                if len(game_name) <= 47:
                    return f'{game_name} (Disc {disc_number})'
                else:
                    return f'{game_name[:47]} (Disc {disc_number})'
            else:
                if len(game_name) <= 47:
                    return game_name
                else:
                    return f'{game_name[:47]}'
        else:
            return game_name

    return ''
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to get the game name from the cue sheet (using the binmerge script)
def _get_game_name_from_cue(cue_path, include_track):
	cue_content = read_cue_file(cue_path)
	if cue_content != []:
		game_name = basename(cue_content[0].filename)
		if not include_track:
			if 'Track' in game_name:
				game_name = game_name[:game_name.rfind('(', 0) -1]
		return splitext(game_name)[0]
	return ''
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to get the unique game id from the bin file
def _get_game_id(bin_file_path):
	game_disc_collection = _get_disc_collection(bin_file_path)
	return game_disc_collection[0].replace('_', '-').replace('.', '').strip() if game_disc_collection else None
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to get the unique game id from the bin file
def _get_disc_collection(bin_file_path):
	game_disc_collection = []
	line = ''
	lines_checked = 0
	if exists(bin_file_path):
		with open(bin_file_path, 'rb') as bin_file:
			while line != None and lines_checked < 300:
				try:
					line = str(next(bin_file))
					if line != None:
						lines_checked += 1
						for region_code in REGION_CODES:
							if region_code in line:
								start = line.find(region_code)
								game_id = line[start:start + 11].replace('.', '').strip()
								if game_id not in game_disc_collection:
									game_disc_collection.append(game_id)
								else:
									raise StopIteration
				except StopIteration:
					break
	return game_disc_collection
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function that gets the disc number (using data from redump)
def _get_disc_number(game_id):
	response = select(f'''SELECT disc_number FROM games WHERE game_id = "{game_id.replace('-','_')}";''')
	if response is not None and response != []:
		return response[0][0]
	return 0
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to copy the game front cover if it is available
def _copy_game_cover(output_path, game_id, game_name):
	response = select(f'''SELECT id FROM covers WHERE game_id = "{game_id.replace('-','_')}";''')
	if response is not None and response != []:
		row_id = response[0][0]
		image_out_path = join(output_path, f'{game_name}.bmp')
		extract_game_cover_blob(row_id, image_out_path)
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to create the global game list
def _create_game_list(selected_path):
	global game_list
	game_list = []

	# Get all of the sub-dirs from the selected directory
	subfolders = [f.name for f in scandir(selected_path) if f.is_dir() and not f.name.startswith('.')]

	# If the user has selected a single directory with no sub-dirs
	if not (subfolders):
		subfolders = [selected_path]

	for subfolder in subfolders:
		_update_window()

		if subfolder != "System Volume Information":
			game_id = None

			game_directory_path = join(selected_path, subfolder)

			# Get the cue_sheet for the game (there could be more than 1 game in the directory)
			cue_sheets = [f for f in listdir(game_directory_path) if f.lower().endswith('.cue') and not f.startswith('.')]

			for cue_sheet in cue_sheets:
				cue_sheet_path = join(game_directory_path, cue_sheet)
				game_name_from_cue = _get_game_name_from_cue(cue_sheet_path, False)

				# Check if the game directory already contains a bmp cover image
				cover_art_path = join(game_directory_path, cue_sheet[:-3])
				cover_art_present = exists(f'{cover_art_path}bmp') or exists(f'{cover_art_path}BMP')

				# Check if the game directory already contains a cu2 file
				cu2_present = exists(join(selected_path, subfolder, f'{cue_sheet[-3]}cu2'))

				# Try and get the unique game_id from the first bin file
				bin_files = read_cue_file(cue_sheet_path)
				if bin_files:
					game_id = _get_game_id(bin_files[0].filename)

				# Try and get the disc number (using data from redump)
				disc_number = 0
				disc_collection = []
				if game_id:
					disc_number = _get_disc_number(game_id)
					disc_collection = _get_disc_collection(join(game_directory_path, f'{game_name_from_cue}.bin'))

				# Create the cue_sheet object
				the_cue_sheet = Cuesheet(cue_sheet, cue_sheet_path, game_name_from_cue)

				# Add each of the bin_file objects to the cue_sheet object
				for bin_file in bin_files:
					the_cue_sheet.add_bin_file(Binfile(basename(bin_file.filename), bin_file.filename))

				# Create the game object
				the_game = Game(subfolder, selected_path, game_id, disc_number, disc_collection, the_cue_sheet, cover_art_present, cu2_present)

				# Add the game to the global game_list
				game_list.append(the_game)
				_print_game_details(the_game)

	game_list.sort(key=lambda game: game.cue_sheet.game_name, reverse=False)
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to print the game details to the console for debugging purposes
def _print_game_details(game):
	print(f'game directory: {game.directory_name}')
	print(f'game path: {game.directory_path}')
	print(f'game id: {game.id}')
	print(f'disc number: {game.disc_number}')

	if game.disc_collection:
		print(f'disc collection: {game.disc_collection}')

	print(f'game cover_art_present: {game.cover_art_present}')
	print(f'game cu2_present: {game.cu2_present}')
	print(f'cue_sheet file_name: {game.cue_sheet.file_name}')
	print(f'cue_sheet file_path: {game.cue_sheet.file_path}')
	print(f'cue_sheet game_name: {game.cue_sheet.game_name}')

	bin_files = game.cue_sheet.bin_files
	print(f'number of bin files: {len(bin_files)}')
	#for bin_file in bin_files:
		#print(f'bin_file file_name: {bin_file.file_name}')
		#print(f'bin_file file_path: {bin_file.file_path}')
	print()
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to parse the game list and displays the results in a message dialog
def _parse_game_list():
	_create_game_list(src_path.get())
	games_without_cover = []
	multi_bin_games = []
	invalid_named_games = []
	unidentified_games = []

	multi_discs = []
	multi_disc_games = []

	for game in game_list:
		_update_window()
		bin_files = game.cue_sheet.bin_files

		if game.id is None:
			unidentified_games.append(game)

		if not game.cover_art_present:
			if game.disc_number != None:
				if int(game.disc_number) < 2:
					games_without_cover.append(game)

		if _is_multi_disc(game):
			multi_discs.append(game)
			if int(game.disc_number) == 1:
				multi_disc_games.append(game)

		if len(bin_files) > 1:
			multi_bin_games.append(game)

		if len(game.cue_sheet.game_name) > MAX_GAME_NAME_LENGTH or '.' in game.cue_sheet.game_name:
			invalid_named_games.append(game)

	progress_bar_indeterminate.stop()
	_update_progress_bar_2(0)

	md = MessageDialog(f'''Total Discs Found: {len(game_list)} \nMulti-Disc Games: {len(multi_disc_games)} \nUnidentfied Games: {len(unidentified_games)} \nMulti-bin Games: {len(multi_bin_games)} \nMissing Covers: {len(games_without_cover)} \nInvalid Game Names: {len(invalid_named_games)}''', title='Game Details', width=650, padding=(20, 20))
	md.show()

	_display_game_list()
	_update_window()

	if multi_bin_games:
		merge_bin_files.set(True)
		force_cu2.set(True)
	if games_without_cover:
		add_cover_art.set(True)
	if invalid_named_games:
		validate_game_name.set(True)

	print()
	print('multi-discs:')
	for game in multi_discs:
		print(game.id)

	print()
	print('multi-disc games:')
	for game in multi_disc_games:
		print(game.id)

	_poo()
# *****************************************************************************************************************





# *****************************************************************************************************************
def _poo():

	print()
	print('checking for multi-disc games...\n')

	for game in game_list:
		if int(game.disc_number) == 1:
			print(f'game id: {game.id}')
			print(f'game name: {game.cue_sheet.game_name}')
			print(f'game disc: {game.disc_number}')
			print(f'game collection: {game.disc_collection}')

# *****************************************************************************************************************





# *****************************************************************************************************************
# Update the _display_game_list function to include the new column with relative directory path
def _display_game_list():
	bools = ('No','Yes')
	for count, game in enumerate(game_list):
		game_id = game.id
		game_name = game.cue_sheet.game_name
		disc_number = game.disc_number
		number_of_bins = len(game.cue_sheet.bin_files)
		name_valid = bools[len(game.cue_sheet.game_name) <= MAX_GAME_NAME_LENGTH and '.' not in game.cue_sheet.game_name]
		cu2_present = bools[game.cu2_present]
		bmp_present = bools[game.cover_art_present]
		relative_game_directory = join(Path(game.directory_path).name, game.directory_name)  # Relative directory path
		treeview_game_list.insert(parent='', index=count, iid=count, text='', values=(game_id, game_name, disc_number, number_of_bins, name_valid, cu2_present, bmp_present, relative_game_directory))  # Include the new field
# *****************************************************************************************************************


# *************************************************
# GUI FUNCTIONS:
# *************************************************


# *****************************************************************************************************************
# Function is a hack to hide the hidden files/directories in the file browser dialog
def _prevent_hidden_files(window):
	try:
		# This will throw a TclError, so we need to catch it (It will initialise the window without displaying it)
		try:
			window.tk.call('tk_getOpenFile', '-foobarbaz')
		except TclError:
			pass
		window.tk.call('set', '::tk::dialog::file::showHiddenBtn', '1')
		window.tk.call('set', '::tk::dialog::file::showHiddenVar', '0')
	except:
		pass
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to update the progress bar
def _update_progress_bar(value):
	progress_bar['value'] = value
	if window is not None:
		window.update()
	sleep(0.1)
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to update the intermediate progress bar
def _update_progress_bar_2(value):
	progress_bar_indeterminate['value'] = value
	if window is not None:
		window.update()
	sleep(0.1)
# *****************************************************************************************************************


# *****************************************************************************************************************
# Function to update the main ui window
def _update_window():
	if window is not None:
		window.update()
	sleep(0.02)
# *****************************************************************************************************************


# *****************************************************************************************************************
# Scan button click event
def _scan_button_clicked():
    button_src_scan['state'] = 'disabled'
    progress_bar_indeterminate.start(20)

    _parse_game_list()

    if force_cu2.get() or merge_bin_files.get() or add_cover_art.get() or validate_game_name.get() or auto_rename.get():
        button_start['state'] = 'normal'
    else:
        progress_bar_indeterminate.stop()
        progress_bar['value'] = 100  # Set progress bar to 100% when scan is complete
        label_progress.configure(text=f'{PROGRESS_STATUS} Scan complete')
# *****************************************************************************************************************


# *****************************************************************************************************************
# Browse button click event
def _browse_button_clicked():

	# Open the fieldialog
	selected_path = filedialog.askdirectory(initialdir = '/', title = 'Select Game Directory')

	# Update the label
	src_path.set(selected_path)
	label_src.configure(text = src_path.get())

	if src_path.get() is not None and src_path.get() != '':
		button_src_scan['state'] = 'normal'
	else:
		button_src_scan['state'] = 'disabled'
# *****************************************************************************************************************


# *****************************************************************************************************************
# Start button click event
def _start_button_clicked():
	if not src_path.get() == '':
		button_start['state'] = 'disabled'
		_process_games()
		button_start['state'] = 'normal'
# *****************************************************************************************************************


# *****************************************************************************************************************
# Checkbox change event
def _checkbox_changed():
	global game_list
	if not force_cu2.get() and not merge_bin_files.get() and not add_cover_art.get() and not validate_game_name.get() and not auto_rename.get():
		button_start['state'] = 'disabled'

	if src_path.get() is not None and src_path.get() != '':
		if force_cu2.get() or merge_bin_files.get() or add_cover_art.get() or validate_game_name.get() or auto_rename.get():
			if game_list:
				button_start['state'] = 'normal'
# *****************************************************************************************************************


# *****************************************************************************************************************
def _store_selected_theme(theme_name):
	with open(CONFIG_FILE_PATH, mode="w") as config_file:
		config_file.write(dumps({"theme": theme_name}))
# *****************************************************************************************************************


# *****************************************************************************************************************
def _switch_theme(theme_name):
	style.theme_use(theme_name)
	_store_selected_theme(theme_name)
# *****************************************************************************************************************







# *************************************************
# Run the GUI
# *************************************************

# Configure grid weight for main window
window.grid_columnconfigure(0, weight=1)
window.grid_rowconfigure(0, weight=0)  # Browse frame
window.grid_rowconfigure(1, weight=1)  # Game list frame
window.grid_rowconfigure(2, weight=0)  # Tools frame
window.grid_rowconfigure(3, weight=0)  # Progress frame

#*************************************************************
# Browse Frame
browse_frame = Labelframe(window, text='SD Root', bootstyle="primary")
browse_frame.grid(row=0, column=0, padx=15, pady=10, sticky='nsew')

# Configure grid for browse frame
browse_frame.grid_columnconfigure(0, weight=1)
browse_frame.grid_columnconfigure(1, weight=0)
browse_frame.grid_rowconfigure(0, weight=0)
browse_frame.grid_rowconfigure(1, weight=0)

# Label to display the src path
label_src = Label(browse_frame, text=src_path.get(), borderwidth=2, relief='solid', bootstyle="primary")
label_src.grid(row=0, column=0, padx=15, pady=5, sticky='ew')

# Button to browse the filesystem
button_src_browse = Button(browse_frame, text='Browse', width=10, bootstyle="primary", command=_browse_button_clicked)
button_src_browse.grid(row=0, column=1, padx=15, pady=5, sticky='e')

# Progress bar and scan button
progress_bar_indeterminate = ttk.Floodgauge(
    browse_frame, 
    font=(None, 14, 'bold'), 
    mask='', 
    mode='indeterminate',
    bootstyle="primary"
)
progress_bar_indeterminate.grid(row=1, column=0, padx=15, pady=5, sticky='ew')

button_src_scan = Button(browse_frame, text='Scan', width=10, command=_scan_button_clicked, state=DISABLED)
button_src_scan.grid(row=1, column=1, padx=15, pady=5, sticky='e')

#*************************************************************
# Game List Frame
game_list_frame = Labelframe(window, text='Files', bootstyle="primary")
game_list_frame.grid(row=1, column=0, padx=15, pady=10, sticky='nsew')

# Configure grid for game list frame
game_list_frame.grid_columnconfigure(0, weight=1)
game_list_frame.grid_columnconfigure(1, weight=0)
game_list_frame.grid_rowconfigure(0, weight=1)

# Treeview setup
treeview_game_list = Treeview(game_list_frame, bootstyle='primary')
treeview_game_list['columns'] = ('ID', 'Name', 'Disc Number', 'Bin Files', 'Name Valid', 'CU2', 'BMP', 'Directory')
treeview_game_list.column('#0', width=0, stretch=NO)
treeview_game_list.column('ID', anchor=CENTER, width=75)
treeview_game_list.column('Name', anchor=CENTER, width=350)
treeview_game_list.column('Disc Number', anchor=CENTER, width=81)
treeview_game_list.column('Bin Files', anchor=CENTER, width=60)
treeview_game_list.column('Name Valid', anchor=CENTER, width=75)
treeview_game_list.column('CU2', anchor=CENTER, width=40)
treeview_game_list.column('BMP', anchor=CENTER, width=40)
treeview_game_list.column('Directory', anchor=CENTER, width=200)

treeview_game_list.heading('#0', text='', anchor=CENTER)
treeview_game_list.heading('ID', text='ID', anchor=CENTER)
treeview_game_list.heading('Name', text='Name', anchor=CENTER)
treeview_game_list.heading('Disc Number', text='Disc Number', anchor=CENTER)
treeview_game_list.heading('Bin Files', text='Bin Files', anchor=CENTER)
treeview_game_list.heading('Name Valid', text='Name Valid', anchor=CENTER)
treeview_game_list.heading('CU2', text='CU2', anchor=CENTER)
treeview_game_list.heading('BMP', text='BMP', anchor=CENTER)
treeview_game_list.heading('Directory', text='Directory', anchor=CENTER)

treeview_game_list.grid(row=0, column=0, padx=(15,0), pady=10, sticky='nsew')

scrollbar_game_list = Scrollbar(game_list_frame, bootstyle="primary-round", orient=ttk.VERTICAL, command=treeview_game_list.yview)
scrollbar_game_list.grid(row=0, column=1, pady=10, sticky='ns')
treeview_game_list.configure(yscroll=scrollbar_game_list.set)

#*************************************************************
# Tools Frame
tools_frame = Labelframe(window, text='Tools', bootstyle="primary")
tools_frame.grid(row=2, column=0, padx=15, pady=10, sticky='ew')

# Configure grid for tools frame - equal columns
tools_frame.grid_columnconfigure(0, weight=1)
tools_frame.grid_columnconfigure(1, weight=1)
tools_frame.grid_columnconfigure(2, weight=1)

# Create frames for each column to center the checkboxes
left_column = ttk.Frame(tools_frame)
left_column.grid(row=0, column=0, rowspan=2, sticky='nsew')
left_column.grid_columnconfigure(0, weight=1)

middle_column = ttk.Frame(tools_frame)
middle_column.grid(row=0, column=1, rowspan=2, sticky='nsew')
middle_column.grid_columnconfigure(0, weight=1)

right_column = ttk.Frame(tools_frame)
right_column.grid(row=0, column=2, rowspan=2, sticky='nsew')
right_column.grid_columnconfigure(0, weight=1)

# Checkboxes for options - now in their respective column frames
checkbox_merge_bin = Checkbutton(left_column, text='Merge Bin Files', bootstyle="round-toggle", takefocus=0, variable=merge_bin_files, command=_checkbox_changed)
checkbox_merge_bin.grid(row=0, column=0, padx=5, pady=2)

checkbox_generate_cue = Checkbutton(left_column, text='CU2 For All', bootstyle="round-toggle", takefocus=0, variable=force_cu2, command=_checkbox_changed)
checkbox_generate_cue.grid(row=1, column=0, padx=5, pady=2)

checkbox_limit_name = Checkbutton(middle_column, text='Fix Invalid Name', bootstyle="round-toggle", takefocus=0, variable=validate_game_name, command=_checkbox_changed)
checkbox_limit_name.grid(row=0, column=0, padx=5, pady=2)

checkbox_auto_rename = Checkbutton(middle_column, text='Auto Rename', bootstyle="round-toggle", takefocus=0, variable=auto_rename, command=_checkbox_changed)
checkbox_auto_rename.grid(row=1, column=0, padx=5, pady=2)

checkbox_add_art = Checkbutton(right_column, text='Add Cover Art', bootstyle="round-toggle", takefocus=0, variable=add_cover_art, command=_checkbox_changed)
checkbox_add_art.grid(row=0, column=0, padx=5, pady=2)

checkbox_create_multi_disc = Checkbutton(right_column, text='Create Multi-Disc', bootstyle="round-toggle", takefocus=0, variable=create_multi_disc, command=_checkbox_changed)
checkbox_create_multi_disc.grid(row=1, column=0, padx=5, pady=2)

#*************************************************************
# Progress Frame
progress_frame = Labelframe(window, text='Progress', bootstyle="primary")
progress_frame.grid(row=3, column=0, padx=15, pady=10, sticky='ew')

# Configure grid for progress frame
progress_frame.grid_columnconfigure(0, weight=1)
progress_frame.grid_columnconfigure(1, weight=0)
progress_frame.grid_rowconfigure(0, weight=0)
progress_frame.grid_rowconfigure(1, weight=0)

# Progress bar
progress_bar = Floodgauge(
    progress_frame, 
    font=(None, 14, 'bold'), 
    mask='', 
    mode='determinate',
    bootstyle="primary"
)
progress_bar.grid(row=0, column=0, padx=15, pady=5, sticky='ew')

# Start button
button_start = Button(progress_frame, text='Start', width=10, command=_start_button_clicked, state=DISABLED)
button_start.grid(row=0, column=1, padx=15, pady=5, sticky='e')

# Progress label
label_progress = Label(progress_frame, text=PROGRESS_STATUS, bootstyle="primary")
label_progress.grid(row=1, column=0, columnspan=2, padx=15, pady=5, sticky='ew')

label_progress.after(1000, ensure_database_exists)

_prevent_hidden_files(window)

# Loop to display/control the GUI
window.mainloop()
# *****************************************************************************************************************
