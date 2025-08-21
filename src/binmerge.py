#!/usr/bin/env python3
#
#  binmerge
#
#  Takes a cue sheet with multiple binary track files and merges them together,
#  generating a corrected cue sheet in the process.
#
#  Copyright (C) 2020 Chris Putnam
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
#
#
#  This code has been modified by LoGi26 (2021) for use with the psio-assist script


from re import search, match
from os import access, R_OK
from os.path import exists, join, dirname, isfile, getsize

# Global variables
error_log_path = None


# **********************************************************************************************************
class Track:
	globalBlocksize = None

	def __init__(self, num, track_type):
		self.num = num
		self.indexes = []
		self.track_type = track_type
		self.sectors = None
		self.file_offset = None

		# All possible blocksize types. You cannot mix types on a disc, so we will use the first one we see and lock it in.
		if not Track.globalBlocksize:
			if track_type in ['AUDIO', 'MODE1/2352', 'MODE2/2352', 'CDI/2352']:
				Track.globalBlocksize = 2352
			elif track_type == 'CDG':
				Track.globalBlocksize = 2448
			elif track_type == 'MODE1/2048':
				Track.globalBlocksize = 2048
			elif track_type in ['MODE2/2336', 'CDI/2336']:
				Track.globalBlocksize = 2336
# **********************************************************************************************************


# **********************************************************************************************************
class File:
	def __init__(self, filename):
		self.filename = filename
		self.tracks = []
		self.size = getsize(filename)
# **********************************************************************************************************


# **********************************************************************************************************
class BinFilesMissingException(Exception):
	pass
# **********************************************************************************************************


# **********************************************************************************************************
def _sectors_to_cuestamp(sectors):
	minutes = sectors / 4500
	fields = sectors % 4500
	seconds = fields / 75
	fields = sectors % 75
	return '%02d:%02d:%02d' % (minutes, seconds, fields)
# **********************************************************************************************************


# **********************************************************************************************************
def _cuestamp_to_sectors(stamp):
	m = match(r'(\d+):(\d+):(\d+)', stamp)
	minutes = int(m.group(1))
	seconds = int(m.group(2))
	fields = int(m.group(3))
	return fields + (seconds * 75) + (minutes * 60 * 75)
# **********************************************************************************************************


# **********************************************************************************************************
# Function that generates a 'merged' cuesheet, that is, one bin file with tracks indexed within.
def _gen_merged_cuesheet(basename, files):
	cuesheet = f'FILE "{basename}.bin" BINARY\n'
	# One sector is (BLOCKSIZE) bytes
	sector_pos = 0
	for f in files:
		for t in f.tracks:
			cuesheet += f'	 TRACK {t.num} {t.track_type}\n'
			for i in t.indexes:
				cuesheet += f'	 INDEX {i["id"]} {_sectors_to_cuestamp(sector_pos + i["file_offset"])}\n'
			sector_pos += f.size / Track.globalBlocksize
	return cuesheet
# **********************************************************************************************************


# **********************************************************************************************************
# Merges files together to new file `merged_filename`, in listed order.
def _merge_files(merged_filename, files):
	if exists(merged_filename):
		_log_error('ERROR', f'Target merged bin path already exists: {merged_filename}')
		return False

	# cat is actually a bit faster, but this is multi-platform and no special-casing
	chunksize = 1024 * 1024
	with open(merged_filename, 'wb') as outfile:
		for f in files:
			with open(f.filename, 'rb') as infile:
				while True:
					chunk = infile.read(chunksize)
					if not chunk:
						break
					outfile.write(chunk)
	return True
# **********************************************************************************************************


# **********************************************************************************************************
# Function to log basic error messages to a file
def _log_error(error_type, error_message):
	if error_log_path is not None:
		try:
			with open(error_log_path, 'a+') as error_log_file:
				error_log_file.write(f'[{error_type}]: {error_message}\n')
		except IOError:
			with open(error_log_path, 'w') as error_log_file:
				error_log_file.write(f'[{error_type}]: {error_message}\n')
# **********************************************************************************************************


# **********************************************************************************************************
# Function to set the error log path
def set_binmerge_error_log_path(log_path):
	global error_log_path
	error_log_path = log_path
# **********************************************************************************************************


# **********************************************************************************************************
def read_cue_file(cue_path):
	files = []
	this_track = None
	this_file = None
	bin_files_missing = False

	f = open(cue_path, 'r')
	for line in f:
		m = search(r'FILE "?(.*?)"? BINARY', line)
		if m:
			this_path = join(dirname(cue_path), m.group(1))
			file_available = (isfile(this_path) or access(this_path, R_OK))

			if not file_available:
				this_path = join(dirname(cue_path), m.group(1).replace(' (Track 01)', ''))
				file_available = (isfile(this_path) or access(this_path, R_OK))
				if not file_available:
					this_path = join(dirname(cue_path), m.group(1).replace(' (Track 1)', ''))
					file_available = (isfile(this_path) or access(this_path, R_OK))

			if not file_available:
				bin_files_missing = True
			else:
				this_file = File(this_path)
				files.append(this_file)

			continue

		m = search(r'TRACK (\d+) ([^\s]*)', line)
		if m and this_file:
			this_track = Track(int(m.group(1)), m.group(2))
			this_file.tracks.append(this_track)
			continue

		m = search(r'INDEX (\d+) (\d+:\d+:\d+)', line)
		if m and this_track:
			this_track.indexes.append({'id': int(m.group(1)), 'stamp': m.group(2), 'file_offset': _cuestamp_to_sectors(m.group(2))})
			continue

	if bin_files_missing:
		_log_error('ERROR', f'file does not exist: {line}')
		return []

	if len(files) == 1:
		next_item_offset = files[0].size // Track.globalBlocksize
		for t in reversed(files[0].tracks):
			t.sectors = next_item_offset - t.indexes[0]["file_offset"]
			next_item_offset = t.indexes[0]["file_offset"]

	return files
# **********************************************************************************************************


# **********************************************************************************************************
def start_bin_merge(cuefile, game_name, outdir):
	cue_map = read_cue_file(cuefile)
	cuesheet = _gen_merged_cuesheet(game_name, cue_map)

	if not exists(outdir):
		_log_error('ERROR', 'Output dir does not exist')
		return False

	new_cue_fn = join(outdir, game_name + '.cue')
	if exists(new_cue_fn):
		_log_error('ERROR', f'Output cue file already exists. Quitting. Path: {new_cue_fn}')
		return False

	if not _merge_files(join(outdir, game_name + '.bin'), cue_map):
		return False

	with open(new_cue_fn, 'w', newline='\r\n') as f:
		f.write(cuesheet)

	return True
# **********************************************************************************************************
