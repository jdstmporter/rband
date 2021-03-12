#!/usr/bin/env python3
'''
Created on 1 Dec 2017

@author: julianporter
'''
from setuptools import setup, Extension
from setuptools.config import read_configuration
import os
import sys
from collections import namedtuple
import subprocess
import tempfile

def getPaths(libs):
    libDir=set()
    incDir=set()
    
    for lib in libs:
        out=subprocess.check_output(['pkg-config','--libs','--cflags',lib]).decode('utf8').split()
        for term in out:
            if term.startswith('-I'): incDir.add(term[2:])
            elif term.startswith('-L'): libDir.add(term[2:])
    return libDir, incDir
    
def checkDependencies(libs):
    src=tempfile.NamedTemporaryFile(suffix ='.c',delete=False)
    name=src.name
    src.file.write(b'int main(int argc,char **argv) { return 0; }')
    src.file.close()
    
    failed=[]
    for lib in libs:
        try:
            subprocess.check_output(['gcc',f'-l{lib}',name]).decode('utf8').split()
            print(f'Library {lib} found')
        except:
            failed.append(lib)
    
    os.unlink(name)
    return failed
       
       
       
    

configuration=read_configuration('setup.cfg')
metadata=configuration['metadata']

def sourceFilesIn(folder,exclude=[]):
    try:
        items=os.listdir(folder)
        return [os.path.join(folder,item) for item in items if item.endswith('.cpp') and item not in exclude]
    except:
        return []

Version = namedtuple('Version',['major','minor','maintenance'])
def processVersion():
    version=metadata['version']
    parts=version.split('.')
    if len(parts)<3: parts.extend([0,0,0])
    return Version(*(parts[:3]))


def buildIncludes():
    try:
        from numpy import get_include
        includes=['/usr/include','/usr/local/include']
        includes.append(f'{get_include()}/numpy')
        return includes
    except:
        print('Numpy does not appear to be installed; please install it, then try again')
        sys.exit(1)

        

def makeExtension(module):
    #print("Making {} with {}".format(module,src))
    
    v=processVersion()
    includes=buildIncludes()
    mv=f'"{v.major}.{v.minor}.{v.maintenance}"'
    
    src=sourceFilesIn('src',exclude=['main.cpp'])
    print(f'Sources are {src}')
    
    return Extension(module,
                    define_macros = [('MAJOR_VERSION', v.major),
                                     ('MINOR_VERSION', v.minor),
                                     ('MAINTENANCE_VERSION', v.maintenance),
                                     ('MODULE_VERSION', mv)],
                    sources = src,
                    language = 'c++',
                    include_dirs=includes,
                    libraries = ['sndfile','rubberband'],
                    library_dirs = ['/usr/lib','/usr/local/lib'],
                    extra_compile_args=['-std=c++17','-DNDEBUG','-O3'])


# src.extend(sourceFilesIn('pcm2mp3-cpp/src/id3',['main.cpp']))

failed = checkDependencies(['sndfile','rubberband'])
if len(failed)>0:
    print('Please install the following libraries:')
    for lib in failed: print(f'    {lib}')
    print('then try again')
    sys.exit(1)
    

coder = makeExtension('rubberband')

with open('README.rst') as readme:
    longDescription = readme.read()

setup (
    ext_modules = [coder],
    long_description = longDescription, 
    entry_points = {
        'distutils.commands' : [
            'deepclean = ext.deepClean:DeepClean'
        ]
    }
)

