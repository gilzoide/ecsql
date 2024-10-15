"""
Pack assets into a ZIP file, transforming them if necessary.

Usage:
  pack_assets.py 
"""
import os
import sys
from zipfile import ZipFile


def pack_assets(assets_folder: str, zipname: str):
    with ZipFile(zipname, 'w') as zipfile:
        for dirname, dirs, files in os.walk(assets_folder):
            for filename in files:
                # skip hidden files, like ".DS_Store"
                if filename.startswith("."):
                    continue
                filepath = os.path.join(dirname, filename)
                zipfile.write(filepath)


if __name__ == "__main__":
    assets_folder, zipname = sys.argv[1:3]
    pack_assets(assets_folder, zipname)
