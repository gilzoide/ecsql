"""
Pack assets into a ZIP file, transforming them if necessary.
"""
import os
import re
from zipfile import ZipFile, ZIP_DEFLATED


def process_texture_atlas(path: str) -> str:
    """
    Compacts texture atlas XML files.
    Our parser ignores XML format and simply reads the attributes in order.
    """
    assert(os.path.exists(os.path.splitext(path)[0] + ".png"))
    contents = []
    wanted_content = re.compile(r'name=[^/]+')
    with open(path, "r") as f:
        for line in f:
            if m := wanted_content.search(line):
                contents.append(m.group(0))
    return "\n".join(contents)


ASSET_PROCESSOR = {
    ".xml": process_texture_atlas,
}


def pack_assets(assets_folder: str, zipname: str):
    with ZipFile(zipname, 'w', compression=ZIP_DEFLATED) as zipfile:
        for dirname, dirs, files in os.walk(assets_folder):
            for filename in files:
                # skip hidden files, like ".DS_Store"
                if filename.startswith("."):
                    continue
                filepath = os.path.join(dirname, filename)
                archivepath = os.path.relpath(filepath, assets_folder)
                ext = os.path.splitext(filepath)[1]
                if f := ASSET_PROCESSOR.get(ext):
                    contents = f(filepath)
                    zipfile.writestr(archivepath, contents)
                else:
                    zipfile.write(filepath, archivepath)


if __name__ == "__main__":
    import argparse

    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument("assets_folder", help="Path to the root of the assets folder")
    argparser.add_argument("zip_name", help="Path to the generated zip file")
    args = argparser.parse_args()
    pack_assets(args.assets_folder, args.zip_name)
