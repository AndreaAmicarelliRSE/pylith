#!@INTERPRETER@
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
#  PyLith by Charles A. Williams, Brad Aagaard, and Matt Knepley
#
#  Copyright (c) 2004-2006 Rensselaer Polytechnic Institute
#
#  Permission is hereby granted, free of charge, to any person obtaining
#  a copy of this software and associated documentation files (the
#  "Software"), to deal in the Software without restriction, including
#  without limitation the rights to use, copy, modify, merge, publish,
#  distribute, sublicense, and/or sell copies of the Software, and to
#  permit persons to whom the Software is furnished to do so, subject to
#  the following conditions:
#
#  The above copyright notice and this permission notice shall be
#  included in all copies or substantial portions of the Software.
#
#  THE  SOFTWARE IS  PROVIDED  "AS  IS", WITHOUT  WARRANTY  OF ANY  KIND,
#  EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
#  MERCHANTABILITY,    FITNESS    FOR    A   PARTICULAR    PURPOSE    AND
#  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
#  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
#  OF CONTRACT, TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN CONNECTION
#  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#

# 


__requires__ = "PyLith"

# main

if __name__ == "__main__":
    
    # re-create the PYTHONPATH at 'configure' time
    import os.path, sys, site
    path = '@PYTHONPATH@'.split(':')
    path.reverse()
    for directory in path:
        if directory:
            directory = os.path.abspath(directory)
            sys.path.insert(1, directory)
            site.addsitedir(directory)

    from pylith3d.PyLith import PyLith
    from pyre.applications import start
    start(applicationClass=PyLith)
    

# version
__id__ = "$Id: pylith3dapp.py,v 1.2 2005/03/11 02:30:46 willic3 Exp $"

#  End of file 
