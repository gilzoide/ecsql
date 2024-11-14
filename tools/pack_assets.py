"""
Pack assets into a ZIP file, transforming them if necessary.
"""
import os
from zipfile import ZipFile


def pack_assets(assets_folder: str, zipname: str):
    with ZipFile(zipname, 'w') as zipfile:
        for dirname, dirs, files in os.walk(assets_folder):
            for filename in files:
                # skip hidden files, like ".DS_Store"
                if filename.startswith("."):
                    continue
                filepath = os.path.join(dirname, filename)
                archivepath = os.path.relpath(filepath, assets_folder)
                zipfile.write(filepath, archivepath)


if __name__ == "__main__":
    import argparse

    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument("assets_folder", help="Path to the root of the assets folder")
    argparser.add_argument("zip_name", help="Path to the generated zip file")
    args = argparser.parse_args()
    pack_assets(args.assets_folder, args.zip_name)
