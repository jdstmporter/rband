'''
Created on 5 Apr 2020

@author: julianporter
'''

from setuptools import Command
from os import listdir, remove, walk
from os.path import isdir
from shutil import rmtree
import re
from itertools import chain

class DeepClean(Command):
    '''Thorough clean of the package, removing various build directories'''
    
    BaseDirs=['build','dist']
    description = '''Deeper clean, removing various build directories'''
    user_options = []
    
    def initialize_options(self):
        pass
        
    
    def finalize_options(self):
        pass
        
        
    def run(self):
        allFiles = listdir('.')
        dirs = [x for x in DeepClean.BaseDirs if x in allFiles]
        egg = [x for x in allFiles if re.match('^.*\.egg-info$',x)] 
        dirs.extend(egg)
        
       
        print('Removing build directories')
        for d in dirs:
            print(f'Attempting to remove {d}')
            try:
                if isdir(d):
                    pass
                    rmtree(d)
                else:
                    pass
                    remove(d)
            except Exception as e:
                print(f'Could not remove {d} : {e}')
                
        
        walked = [(a,c) for a,_,c in walk('.')]
        paths =chain(*[[f'{a}/{cc}' for cc in c] for a,c in walked])
        objs = [x for x in paths if re.match('^.*\.(o|so)$',x)]
        
        print('Removing object code files')
        for obj in objs:
            print(f'Attempting to remove {obj}')
            try:
                pass
                remove(obj)
            except Exception as e:
                print(f'Could not remove {obj} : {e}')
        
        
   
    
