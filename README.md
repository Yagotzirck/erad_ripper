# erad_ripper
An archive extractor tool for the DOS game Eradicator (1996). 

## Usage
	erad_ripper <file>
Where \<file> is one of the following 3 files passed as an argument (in Windows you can also use the drag-and-drop method):
- MAIN.RID
- AMBIENT.RAW
- MUSIC.RAW

MAIN.RID is the game's archive containing pretty much everything; the 2 .RAW files are the background music files used in-game and for the menu, respectively.   

Once the extractor has done its thing, you should have the extracted resources in a subfolder
located in the same path as the file you passed as a parameter (if you didn't use the drag-and-drop method, you should see the subfolder's path
printed as soon as the extraction is done, just in case).

## Extracted material
- Textures
- Sprites
- Various graphics (fonts, menus, HUDs, etc)
- Palettes
- Sounds
- Music (AMBIENT.RAW and MUSIC.RAW)

All picture-related material is extracted as .TGA pictures, which can be viewed fine with
IrfanView (albeit without transparency), SLADE3, MtPaint, and possibly other image viewers I'm unaware of;
if that's still a problem for you, Ken Silverman's PNGOUT is your friend :)

As a last note, I couldn't find anything that associates each picture to its respective palette; if I were to take a guess,  
it looks like the game uses a fixed palette + graphics combination for each mission set, which is the reason why you
must select the palette manually at the beginning of the extraction process. As tedious as it may be, I couldn't come up with any other solution, sorry about that.
