from setuptools import setup

APP = ['src/psio_assist.py']
DATA_FILES = []
OPTIONS = {
    'argv_emulation': True,
    'packages': ['ttkbootstrap', 'pathlib2'],
}

setup(
    app=APP,
    data_files=DATA_FILES,
    options={'py2app': OPTIONS},
    setup_requires=['py2app'],
)