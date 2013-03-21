
CONTENTS OF THIS FILE
---------------------

 * summary
 * requirements


SUMMARY
-------

Document Solution Pack for Islandora

Loads all required Fedora Objects, and creates an empty collection
object to accept ingested documents of types supported by JODCOnverter
 (currently : OpenDocument, PDF, RTF, HTML, Word, Excel, PowerPoint, and Flash).
 
A PDF version is attempted to be created and stored of the file
  (although for spreadsheets this does look a bit odd).

A thumbnail and preview is also created from the created PDF if successful.

REQUIREMENTS
------------

ImageMagick: http://www.imagemagick.org/script/index.php

JODconverter: http://code.google.com/p/jodconverter/
  - JODConverter requires an instance of either 
    openoffice (http://www.openoffice.org/),
    or libreoffice (http://www.libreoffice.org)
    running in headless mode.
    
e.g. /usr/bin/soffice --nologo --nofirststartwizard --headless --norestore --invisible "--accept=socket,host=localhost,port=2002,tcpNoDelay=1;urp;" &

Apache Tika
  - should you wish to extract the text from docs.
   This can be useful to provide text searching within the document but it not mandatory.
   http://tika.apache.org/
