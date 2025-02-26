# Psio-Assist-EX
Python scripts with a basic GUI to prepare PlayStation 1 bin/cue games for use with a PSIO device.<br>
Works on Linux, Mac and Windows.<br>

![alt text](https://github.com/logi-26/psio-assist/blob/v0.2/image.png?raw=true)

**This application:**<br/>
Organizes and standardizes PlayStation 1 games into a format acceptable by the PSIO device. It performs the following tasks:<br/>
- Merges multi-bin games into a single bin file.<br/>
- Converts cue files into cu2 format.<br/>
- Works in batch mode on all bin/cue files in the selected directory and any sub-directories.<br/>
- Ensures that game names are not greater than 60 characters and do not contain periods or slashes.<br/>
- Generates the MULTIDISC.LST file for multi-disc games.<br/>
- Does not modify the original bin/cue files.<br/>
- Downloads cover images for games that do not have them in their directories. If a cover image is not found, it ignores the game and continues processing the others.<br/>

## Info
This application uses the following scripts:<br/>
**binmerge**<br/>
https://github.com/putnam/binmerge <br/>
**cue2cu2**<br/>
https://github.com/NRGDEAD/Cue2cu2

## Dependencies
This project requires Python 3 and the following Python packages:
- `ttkbootstrap`
- `pathlib2`

### Installation Steps
1. **Install Python 3**:
   - Download and install Python 3 from the official website: https://www.python.org/downloads/
   - Ensure Python 3 is added to your system PATH.

2. **Install pip**:
   - Pip is usually included with Python 3. To check if pip is installed, run:
     ```bash
     pip --version
     ```
   - If pip is not installed, you can install it by following the instructions here: https://pip.pypa.io/en/stable/installation/

3. **Set up a virtual environment**:
   - It is recommended to use a virtual environment to manage dependencies. To create and activate a virtual environment, follow these steps:
     - Create a virtual environment:
       ```bash
       python3 -m venv venv
       ```
     - Activate the virtual environment:
       - On Windows:
         ```bash
         venv\Scripts\activate
         ```
       - On macOS and Linux:
         ```bash
         source venv/bin/activate
         ```
       - On Linux (alternative method):
         ```bash
         . venv/bin/activate
         ```
   - For more information on virtual environments, refer to the official documentation: https://docs.python.org/3/library/venv.html

4. **Install the required Python packages**:
   - With the virtual environment activated, run:
     ```bash
     pip install ttkbootstrap pathlib2
     ```

## Usage
1. **Run the application**:
   - Ensure you have activated your virtual environment.
   - Navigate to the `src` directory where `psio_assist.py` is located.
   - Run the script using Python:
     ```bash
     python3 psio_assist.py
     ```

2. **Using the GUI**:
   - Click on the **Browse** button and select the directory that contains your bin/cue files.
   - Click on the **Scan** button to scan the games inside the selected directory.
   - Select the desired process options.
     *It is recommended to have at least the bin files merged due to PSIO compatibility issues.*
   - Click on the **Start** button to process the desired options.
   - The progress bar will display the progress of the application.
