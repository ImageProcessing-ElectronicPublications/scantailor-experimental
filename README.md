# ScanTailor-Experimental

Based on Scan Tailor - [scantailor.org](https://web.archive.org/web/20210304010738/http://scantailor.org/)

![ScanTailor logo](illustrations/1200px-Scan_Tailor_-_Logo.svg_-300x288.png) 


## About ##

ScanTailor-Experimental is an interactive post-processing tool for scanned pages. 
It performs operations such as:
  - [page splitting](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Split-Pages), 
  - [deskewing](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Deskew), 
  - [adding/removing borders](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Page-Layout), 
  - [selecting content](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Select-Content) 
  - ... and others. 
  
You give it raw scans, and you get pages ready to be printed or assembled into a PDF 
  or [DJVU](http://elpa.gnu.org/packages/djvu.html) file. Scanning, optical character recognition, 
  and assembling multi-page documents are out of scope of this project.

ScanTailor-Experimental is [Free Software](https://www.gnu.org/philosophy/free-sw.html) (which is more than just freeware). 
  It’s written in C++ with Qt and released under the General Public License version 3. 
  We develop both Windows and GNU/Linux versions.

## History and Future

This project started in late 2007 and by mid 2010 it reached production quality. 

In 2014, the original developer [Joseph Artsimovich](https://github.com/Tulon) stepped aside, 
and [Nate Craun](https://natecraun.net/) ([@ncraun](https://github.com/ncraun)) 
  took over as the [new maintainer](https://web.archive.org/web/20140428040025/scantailor.org/2014/04/06/new-maintainer.html).

In 2023 ScanTailor-Experimental fork began its own life.

For information about original Scan Tailor roadmap, please see the 
  [Roadmap](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Roadmap-1.0) wiki entry.
  
For any suggested changes or bugs, please consult the [Issues](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/issues) tab.

## Usage

ScanTailor-Experimental is being used not just by enthusiasts, but also by libraries and other institutions. 
  Examples of books processed with original Scan Tailor can be found on Google Books and the Internet Archive. 
  - [Prolog for Programmers](https://sites.google.com/site/prologforprogrammers/the-book). The 47.3MB pdf is the original, 
    and the 3.1MB pdf is after using Scan Tailor. The OCR, Chapter Indexing, JBIG2 compression, and PDF Binding were not 
    done with Scan Tailor, but all of the scanned image cleanup was. [[1](https://web.archive.org/web/20140421083201/scantailor.org/downloads/)]
  - [Oakland Township: Two Hundred Years](http://books.google.com/books?printsec=frontcover&id=o4Q2OlVl61MC) 
      by Stuart A. Rammage (also available: volumes 2, 3, 4.1, 4.2, 5.1, and 5.2) [[2](http://www.diybookscanner.org/forum/viewtopic.php?t=435)]
  - [Herons and Cobblestones](http://books.google.com/books?printsec=frontcover&id=o4Q2OlVl61MC): A History of Bethel and the Five Oaks Area of Brantford Township, 
      County of Brant by the Grand River Heritage Mines Society [[2](http://www.diybookscanner.org/forum/viewtopic.php?t=435)]


## Installation and Tips
  
[Scanning Tips](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Tips-for-Scanning), 
  [Quick-Start-Guide](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Quick-Start-Guide), and complete 
  [Usage Guide](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/User-Guide), including installation information 
  (via the [installer](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/User-Guide#installation-and-first-start) or 
  [building from from source](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/Building-from-Source-Code-on-Linux-and-Mac-OS-X))
  can be found in the [wiki](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/wiki/)!
  
### Installation on Windows

On Windows 10 1809 or higher to install Scantailor-Experimental just use command:

``` cmd
winget install "IPEP.Scantailor-Experimental"
```

You can also download binaries from [Release page](https://github.com/ImageProcessing-ElectronicPublications/scantailor-experimental/releases).

## Additional Links 

- [Linux Guide to Book Scanning](https://natecraun.net/articles/linux-guide-to-book-scanning.html)
- [DIY Book Scanner Forum](http://diybookscanner.org/forum/)
- [Scan Tailor Subforum](http://diybookscanner.org/forum/viewforum.php?f=21) from the above.
