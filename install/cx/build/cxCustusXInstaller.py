#!/usr/bin/env python

#####################################################
# Unix jenkins script
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2013.09.05
#
# Description:
#
#
#####################################################

import logging
import time    
import subprocess
import sys
import argparse        
import glob
import platform
import shlex
import datetime

from cx.utils.cxShell import *
from cx.utils.cxPrintFormatter import PrintFormatter
import cx.utils.cxUtilities
import cx.utils.cxSSH
import cxInstallData
import cxComponents
import cxComponentAssembly

class CustusXInstaller:
    '''
    Given a compiled CustusX package,
    Functionality for installing and publishing releases.
    '''
    def __init__(self):
        pass
    def setRootDir(self, root_dir):
        'root dir for user data'
        self.root_dir = root_dir
        if platform.system() == 'Darwin':
            self.install_root = '/Applications'
        if (platform.system() == 'Linux'):
            self.install_root = '%s/Installed' % self.root_dir
        if(platform.system() == 'Windows'):
            self.install_root = '%s\\Installed' % self.root_dir

    def setTargetPlatform(self, target_platform):
        self.target_platform = target_platform  

    def getInstalledRoot(self):
        '''
        Return path to base of the installation 
        performed by installPackage().
        '''
        return self.install_root
    
    def setInstallerPath(self, path):
        'set location of installer package (dmg, msi, gz) that should be installed'
        self.installer_path = path
        
    def setSourcePath(self, source_custusx_path):
        'location of source code root'
        self.source_custusx_path = source_custusx_path      
#        self.source_custusxsetup_path = source_custusxsetup_path      
        
    def removePreviousJob(self):
        'remove all stuff from previous run of the installer'
        PrintFormatter.printHeader('Removing files from previous install', 3);
        shell.rm_r('%s/CustusX' % self.install_root)
        shell.rm_r('%s/CustusX*.%s' % (self.installer_path, self._getInstallerPackageSuffix()))
        shell.removeTree('%s/temp/Install' % self.root_dir)
        shell.removeTree('%s/CustusX' % self.install_root)
        
    def createReleaseFolder(self):
        '''
        Create a folder containing all the files required for a Release.
        Ready to be moved to a distribution server.
        '''
        PrintFormatter.printHeader('create local release folder', level=2)
        targetPath = self._generateReleaseFolderName()
        PrintFormatter.printInfo('Creating folder %s' % targetPath)
        #shell.run('mkdir -p %s' % targetPath)
        shell.makeDirs(targetPath)
        installerFile = self.findInstallerFile()
        self._copyFile(installerFile, targetPath)
#        self.copyReleaseFiles(targetPath)                        
        return targetPath

    def _removeLocalTags(self):    
        '''
        Remove local tags,
        this removes jenkins tags that hides our own.
        # http://stackoverflow.com/questions/1841341/remove-local-tags-that-are-no-longer-on-the-remote-repository
        '''
        PrintFormatter.printInfo('Removing local git tags ...')
        shell.changeDir(self.source_custusx_path)
        # shell.run('git tag -l | xargs git tag -d') no good on windows
        # shell.run('git fetch')
        shell.run('git fetch origin --prune --tags')
        
    def _getDateString(self):
        return '%s' % datetime.date.today().isoformat()
        
    def getTaggedFolderName(self):
        '''
        Generate a folder name based on the current git tag.
        If not on a tag, create something similar but .dirty.
        Use the CustusX tags to generate - we assume that all
        repos are in syn i.e. have the same tag.
        
        Examples: v15-04                (version 15-04)
                  v15-04-245.dtq2-dirty (untagged git checkout)
        '''
        shell.changeDir(self.source_custusx_path) 
        self._removeLocalTags()    
        
        output = shell.evaluate('git describe --tags --exact-match')
        if output:
            name = output.stdout.strip() 
        else:
            output = shell.evaluate('git describe --tags')
            name = output.stdout.strip() 
            name = '%s.%s-dirty' % (self._getDateString(), name)
        #name = 'CustusX_%s' % name
        return name

    def _generateReleaseFolderName(self):
        '''
        Generate a name for the folder to insert release files into.
        This is a temporary folder that can be used to publish the release.
        '''
        taggedFolderName = self.getTaggedFolderName()
        targetPath = '%s/Release/CustusX_%s' % (self.installer_path, taggedFolderName)
        return targetPath
    
    def copyReleaseFiles(self, targetPath):
        'Copy files into release folder in addition to the installer.'
        source = self.source_custusxsetup_path
        self._copyFile('%s/doc/ChangeLog.rtf' % source, targetPath)
        self._copyFile('%s/doc/CustusX_Specifications.pdf' % source, targetPath)
        self._copyFile('%s/doc/CustusX_Tutorial.pdf' % source, targetPath)
        if platform.system() == 'Darwin':
            self._copyFolder('%s/install/Apple/drivers' % source, targetPath)
            self._copyFile('%s/install/Apple/apple_install_readme.rtf' % source, targetPath)
        if platform.system() == 'Linux':
            linux_distro = 'Ubuntu'
#            self._copyFolder('%s/install/Linux/script/vga2usb' % source, targetPath)
            if linux_distro == 'Ubuntu':
                self._copyFolder('%s/install/Linux/script/Ubuntu12.04' % source, targetPath)
            if linux_distro == 'Fedora':
                self._copyFolder('%s/install/Linux/script/Fedora14' % source, targetPath)
        if platform.system() == 'Windows':
            self._copyFile('%s/install/Windows/Windows_Install_ReadMe.rtf' % source, targetPath)
        
    def _copyFolder(self, source, targetPath):
        targetFolder = os.path.split(source)[1]
        fullTargetPath = '%s/%s' % (targetPath, targetFolder)
        shutil.rmtree(fullTargetPath, True)
        shutil.copytree(source, fullTargetPath)
        PrintFormatter.printInfo("copied folder %s into %s" % (source, targetPath))
        
    def _copyFile(self, source, targetPath):
        shutil.copy2(source, targetPath)
        PrintFormatter.printInfo("copied file %s into %s" % (source, targetPath))

#        self.cxInstaller.publishReleaseFolder(source, targetFolder)  
#    def publishReleaseFolder(self, path, target):
    def publishReleaseFolder(self, source, targetFolder, target):
        '''
        Copy a release folder to server
        '''
        PrintFormatter.printHeader('copy/publish package to medtek server', level=2)
        remoteServer = target.server
        remoteServerPath = target.path
#        remoteServer = "medtek.sintef.no"
#        remoteServerPath = "/Volumes/MedTekEksternDisk/Software/CustusX/AutomatedReleases"
        
#        targetFolder = os.path.split(path)[1]
        target_path = '%s/%s/%s' % (remoteServerPath, targetFolder, self._getUserFriendlyPlatformName())
        PrintFormatter.printInfo('Publishing contents of [%s] to remote path [%s]' % (source, target))
        targetBasePath = '%s/%s' % (remoteServerPath, targetFolder) # need to create parent folder explicitly

        transfer = cx.utils.cxSSH.RemoteFileTransfer()
        transfer.connect(remoteServer, target.user)
        transfer.remote_mkdir(targetBasePath)
        transfer.copyFolderContentsToRemoteServer(source, target_path);
        transfer.close()
        
    def _getUserFriendlyPlatformName(self):
        'generate a platform name understandable for users.'
        name = self.target_platform.get_target_platform()
        return name.title()

    def installPackage(self):
        '''
        Install the package to the default location on this machine,
        based on root_dir if necessary.
        '''
        PrintFormatter.printHeader('Install package', level=3)
        file = self.findInstallerFile()
        PrintFormatter.printInfo('Installing file %s' % file)
        self._installFile(file)
        
    def findInstallerFile(self):
        '''
        Find the full name of the installer file.
        '''
        pattern = self._getInstallerPackagePattern()
        PrintFormatter.printInfo('Looking for installers with pattern: %s' % pattern)
        files = glob.glob(pattern)
        cx.utils.cxUtilities.assertTrue(len(files) == 1,
                        'Found %i install files, requiring 1: \n pattern: %s\n Found:\n %s' % 
                        (len(files), pattern, ' \n'.join(files)))
        file = files[0]
        return file

    def _installFile(self, filename):
        if platform.system() == 'Darwin':
            self._installDMG(filename)
        if platform.system() == 'Linux':
            self._installLinuxZip(filename)
        if platform.system() == 'Windows':
            self._installWindowsNSISExe(filename)
    
    def _getInstallerPackagePattern(self):
        suffix = self._getInstallerPackageSuffix()
        return '%s/CustusX*.%s' % (self.installer_path, suffix)
        
    def _getInstallerPackageSuffix(self):
        if platform.system() == 'Darwin':
            return 'dmg'
        if platform.system() == 'Linux':
            return 'tar.gz'
        if platform.system() == 'Windows':
            return 'exe'
        cx.utils.cxUtilities.assertTrue(False, 'suffix not found for OS=%s' % platform.system())
        
    def _installWindowsNSISExe(self, filename):
        installfolder = '%s\CustusX' % self.install_root
        installfolder = installfolder.replace("/", "\\")
        filename = filename.replace("/", "\\")
        cmd = ["%s" % filename, "/S", "/D=%s" % installfolder]
        shell.run(cmd, convertToString=False)
        PrintFormatter.printInfo('Installed \n\t%s\nto folder \n\t%s ' % (filename, self.install_root))

    def _installLinuxZip(self, filename):
        temp_path = '%s/temp/Install' % self.root_dir
        shell.removeTree(temp_path)
        shell.changeDir(temp_path)
        shell.run('tar -zxvf %s' % (filename))  # extract to path
        corename = os.path.basename(filename).split('.tar.gz')[0]
        unpackedfolder = "%s/%s" % (temp_path, corename)
        installfolder = '%s' % self.install_root
        shell.changeDir(installfolder)
        shell.run('cp -r %s/* %s' % (unpackedfolder, installfolder))
        PrintFormatter.printInfo('Installed \n\t%s\nto folder \n\t%s ' % (filename, installfolder))

    def _installDMG(self, dmgfile, pkgName=None):
        '''
        Install the given pkg inside the dmg file.
        '''
        path = os.path.dirname(dmgfile)
        basename = os.path.basename(dmgfile)
        changeDir(path)
        coreName = os.path.splitext(basename)[0]
        if not pkgName:
            pkgName = coreName + '.mpkg'
        PrintFormatter.printInfo("install package %s from file %s" % (pkgName, coreName))
        shell.run('hdiutil attach -mountpoint /Volumes/%s %s' % (coreName, dmgfile))
        target = '/'  # TODO: mount on another (test) volume - this one requires sudo
        shell.run('sudo installer -pkg /Volumes/%s/%s -target %s' % (coreName, pkgName, target))
        shell.run('hdiutil detach /Volumes/%s' % coreName)
        PrintFormatter.printInfo("Installed %s" % pkgName)
