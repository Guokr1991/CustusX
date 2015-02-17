#!/usr/bin/env python

#####################################################
# Unix setup script
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2012.01.19
#
# Description:
#
# Default values:
# - change the class Common
#
# default folder setup
#
# --root----ITK---ITK
#    |       |----build_Debug  
#    |       |----build_Release
#    |       |----build32_Debug
#    |
#    |------VTK---VTK
#    |       |----build_Debug  
#    |       |----build_Release
#    |       |----build32_Debug
#    |
#    |------CustusX---CustusX
#            |--------build_Debug  
#            |--------build_Release
#            |--------build32_Debug
#
#
# old, deprecated structure:
# 
# --root---external---ITK---ITK
#    |         |       |----build_Debug  
#    |         |       |----build_Release
#    |         |       |----build32_Debug
#    |         |
#    |         |------VTK---VTK
#    |                 |----build_Debug  
#    |                 |----build_Release
#    |                 |----build32_Debug
#    |-----working
#              |------CustusX---CustusX
#                      |--------build_Debug  
#                      |--------build_Release
#                      |--------build32_Debug
#             
#################################################             

import subprocess
import optparse
import re
import sys
import os.path
import urllib
import getpass
import platform
import argparse
import pprint

import cx.utils.cxArgParse
import cx.utils.cxUtilities
import cx.utils.cxSSH
        
class Common(object):
    '''
    Container for common data.
    Modify these to change behaviour
    '''
    def __init__(self):
        self.root_dir = None
        self.m32bit = False
        self._initPaths()
        self.main_branch = "master"
        self.static = False # build as shared or static libraries
        self.jom = False
        self.ninja = self.ninja_installed()                
        self.eclipse_version = '3.6'
        self.setBuildType("Debug") 
        self.threads = 1
        self.xcode = False
        self.mBuildSSCExamples = True
        self.mBuildTesting = True
        self.mGEStreamerUseOpenCL = True
        self.mOSX_DEPLOYMENT_TARGET = "10.6" # Deploy for OSX 10.6 Snow Leopard and later
        self.short_pathnames = False
        if (platform.system() == 'Windows'):
            self.ninja = True
            self.short_pathnames = True
            self.mBuildSSCExamples = False
            self.mBuildTesting = True
        if (platform.system() == "Darwin"):
            self.mGEStreamerUseOpenCL = False # Turn off OpenCL for Mac as Jenkins tests are run on olevs mac, and OpenCL code don't work there yet
        self.mBuildExAndTest = False
        self.mCoverage = False
        self.build_developer_doc = False
        self.build_user_doc = False
        self.mGraphviz = False
        self.git_tag = None # if none, use branch master
        self.force_connect_sublibraries = False

        self.publish_release_target                 = cx.utils.cxSSH.RemoteServerID("example.com", "/path/to/folder")
        self.publish_developer_documentation_target = cx.utils.cxSSH.RemoteServerID("example.com", "/path/to/folder")
        self.publish_user_documentation_target      = cx.utils.cxSSH.RemoteServerID("example.com", "/path/to/folder") 

        # for the build of CustusX on the custusx.org website
        user = "custusx"
        server = "mtwiki.sintef.no"
        root_folder = 'uploads/oss_custusx'
        self.publish_release_target                 = cx.utils.cxSSH.RemoteServerID(server, "%s/releases"%root_folder, user)
        self.publish_developer_documentation_target = cx.utils.cxSSH.RemoteServerID(server, "%s/developer_doc"%root_folder, user)
        self.publish_user_documentation_target      = cx.utils.cxSSH.RemoteServerID(server, "%s/user_doc"%root_folder, user) 

        self.gitrepo_internal_site_base = "user@example.com/path/to/folder" #intended for use with "git checkout ssh://%s"
        self.gitrepo_open_site_base = "git@github.com:SINTEFMedtek"  

    def printSettings(self):
        print ''
        print 'Settings:'
        print '    User:', getpass.getuser()
        print '    platform:', platform.system()
        print '    RootDir:', self.getRootDir()
        print '    ExternalDir:', self.getExternalPath()
        print '    WorkingDir:', self.getWorkingPath()
        print '    CMakeGenerator:', self.getCMakeGenerator()
        if('Eclipse' in self.getCMakeGenerator()):
            print '    Eclipse version:', self.getEclipseVersion()
        print '    BuildShared:', self.getBuildShared()
        print '    BuildType:', self.getBuildType()
        print '    BuildExternalsType:', self.getBuildExternalsType()
        print '    BuildTesting:', self.mBuildTesting
        print '    Coverage:', self.mCoverage
        print '    Make dependency graph:', self.mGraphviz
        print '    Threads:', self.threads
        print '    32 bit:', self.m32bit
        print '    git tag:', self.git_tag
        print '    GEStreamerOpenCL:', self.mGEStreamerUseOpenCL
        print '    force_connect_sublibraries:', self.force_connect_sublibraries
        print '    short_pathnames:', self.short_pathnames
        print ''

    def getArgParser_root_dir(self):
        p = cx.utils.cxArgParse.ArgumentParser(add_help=False)
        p.add_argument('--root_dir', default=self.root_dir, help='specify root folder, default=%s' % self.root_dir)
        p.add_argument('--print_control_data', action='store_true', default=False, help='Print all control data at startup')
        return p

    def getArgParser_core_build(self):
        p = cx.utils.cxArgParse.ArgumentParser(add_help=False)
        p.add_argument('-j', '--threads', type=int, default=1, dest='threads', help='Number of make threads')
        p.add_argument('-g', '--git_tag', default=None, metavar='TAG', dest='git_tag', help='Git tag to use when checking out CustusX. None means checkout master branch.')
        p.add_argument('-t', '--build_type', default=self.build_type, dest='build_type', choices=self._getAllowedBuildTypes(), help='Build type')
        p.add_boolean_inverter('--b32', default=self.m32bit, dest='m32bit', help='Build 32 bit.')
        p.add_argument('--main_branch', default=self.main_branch, dest='main_branch', help='Default branch to checkout/pull, for projects not using a custom branch or tag, default=%s. When empty, checkout is skipped.' % self.main_branch)
        p.add_boolean_inverter('--static', default=self.static, dest='static', help='Link statically.')        
        if platform.system() == 'Windows':
            p.add_boolean_inverter('--jom', default=self.jom, dest='jom', help='Use jom to build.')
        p.add_boolean_inverter('--ninja', default=self.ninja, dest='ninja', help='Use ninja to build.')
        if platform.system() == 'Darwin':
            p.add_boolean_inverter('--xcode', default=self.xcode, dest='xcode', help='Generate xcode targets')
        p.add_boolean_inverter('--force_connect_sublibraries', default=self.force_connect_sublibraries, dest='force_connect_sublibraries', help='Force libs such as gestreamer and tsf to be connected to cx, during configuration step.')        
        p.add_boolean_inverter('--short_pathnames', default=self.short_pathnames, dest='short_pathnames', help='Create shorter pathnames where possible. Workaround for the path length limitation on Windows.')
        p.add_boolean_inverter('--graph', default=self.mGraphviz, dest='mGraphviz', help='Make dependency graph.')
        p.add_argument('-ev', '--eclipse_version', default='3.6', dest='eclipse_version', choices=['3.2', '3.3', '3.4', '3.5', '3.6', '3.7', '4.2', '4.3'], help='The Eclipse version number')
        return p

    def getArgParser_extended_build(self):
        p = cx.utils.cxArgParse.ArgumentParser(add_help=False)
        p.add_boolean_inverter('--coverage', default=self.mCoverage, dest='mCoverage', help='gcov code coverage')
        p.add_boolean_inverter('--developer_doc', default=self.build_developer_doc, dest='build_developer_doc', help='Build developer documentation')
        p.add_boolean_inverter('--user_doc', default=self.build_user_doc, dest='build_user_doc', help='Build user documentation')
        return p

    def applyCommandLine(self, arguments):
        arguments = self.getArgParser_root_dir().parse_known_args(args=arguments, namespace=self)[1]
        arguments = self.getArgParser_core_build().parse_known_args(args=arguments, namespace=self)[1]
        arguments = self.getArgParser_extended_build().parse_known_args(args=arguments, namespace=self)[1]
        if self.print_control_data:
            #self.printSettings()
            pprint.pprint(vars(self))
        self.root_dir = os.path.abspath(self.root_dir)
        return arguments

    def getCMakeGenerator(self):
        if self.xcode:
            return "Xcode"
        if self.jom:
            return "NMake Makefiles JOM"
        if self.ninja:
            return 'Eclipse CDT4 - Ninja'
        if platform.system() == 'Windows':
            return 'Eclipse CDT4 - NMake Makefiles' # need to surround with ' ' instead of " " on windows for it to work
        return "Eclipse CDT4 - Unix Makefiles" 
    
    def getEclipseVersion(self):
        return self.eclipse_version
    
    def setBuild32(self, value):
        self.m32bit = value        
                        
    def getTargetPlatform(self):
        return cx.utils.cxUtilities.PlatformInfo(platform.system(), self.m32bit)
    
    def _getAllowedBuildTypes(self):
        return ['Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel']

    def setBuildType(self, value):
        allowedValues = self._getAllowedBuildTypes()
        if value not in allowedValues:
            raise Exception("Error: %s is not a valid build type. Chose one of %s" % (value, allowedValues))
        self.build_type = value
    
    def getBuildShared(self):
        return not self.static # self._buildShared
    def getBuildType(self):
        return self.build_type
    
    def getBuildExternalsType(self):
        if(platform.system() == 'Windows'): # Windows do not allow linking between different build types
            return self.build_type            
        return "Release" # used for all non-cx libs, this because we want speed even in debug...
    
    def setRootDir(self, root_dir):
        if root_dir:
            self.root_dir = root_dir
            
    def getRootDir(self):
        return self.root_dir
    
    def getWorkingPath(self):
        return self.root_dir
    
    def getExternalPath(self):
        return self.root_dir
    
    def setBuildShared(self, value):
        self.static = not value
    
    def getBuildFolder(self):
        retval = []
        self._appendToBuildFolder(retval, self._addLongOrShortPathID("build", "b"))
        self._appendToBuildFolder(retval, self.build_type)
        self._appendToBuildFolder(retval, self._addLongOrShortPathID("static", "s", add_only_if=self.static))
        self._appendToBuildFolder(retval, self._addLongOrShortPathID("32", "32", add_only_if=self.m32bit), delimiter='')
        self._appendToBuildFolder(retval, self._addLongOrShortPathID("xcode", "x", add_only_if=self.xcode))
        self._appendToBuildFolder(retval, self._addLongOrShortPathID("jom", "j", add_only_if=self.jom))
        #self._appendToBuildFolder(retval, self._addLongOrShortPathID("ninja", "n", add_only_if=self.ninja))
        return ''.join(retval)
    
    def _appendToBuildFolder(self, retval, value, delimiter='_'):
        if len(value)==0:
            return
        if len(retval)>0:
            retval.append(delimiter)
        retval.append(value)

    def _addLongOrShortPathID(self, long_name, short_name, add_only_if=True):
        if not add_only_if:
            return ""
        if self.short_pathnames:
            return short_name
        else:
            return long_name
            
    def _initPaths(self):        
        if platform.system() == 'Windows':
            self.root_dir = "C:/Dev/cx"
        else:
            self.root_dir = os.path.expanduser("~") + "/dev/cx" #+ getpass.getuser() - use new default
        
    def _getExternalFolder(self):
        '''external dir: Used as base dir for all externals, such as VTK, ITK, ...'''
        return self._addLongOrShortPathID("external", "ext")

    def _getWorkingFolder(self):
        ''' working dir: Used as base dir for Custus and other of our 'own' projects '''
        return "working"
        
    def getGitTag(self):
        if self.git_tag == "":
            return None
        return self.git_tag

    def ninja_installed(self):
        spawn = cx.utils.cxUtilities.try_module_import('distutils.spawn')
        if not spawn:
            return False
        ninja_found = spawn.find_executable('ninja')
        return ninja_found is not None

# ---------------------------------------------------------

