#!/usr/bin/env python3
import os, shutil, py7zr, pysftp, paramiko
from base64 import b64decode
# register file format at first.
shutil.register_archive_format('7zip', py7zr.pack_7zarchive, description='7zip archive')
shutil.register_unpack_format('7zip', ['.7z'], py7zr.unpack_7zarchive)

rootDir = os.path.dirname(__file__)
projectDir = os.path.join(rootDir, '..')
releaseDir = os.path.join(projectDir, 'bin/Release/netcoreapp3.1')
tempDir = os.path.join(projectDir, 'temp')


def prepereAndUpload(version):
    print('Preping archives')
    version = version.replace('.', '_')
    print(releaseDir)

    archive(version, 'win-x64')
    archive(version, 'linux-x64')
    archive(version, 'osx-x64')
    uploadArchive(['win-x64', 'linux-x64', 'osx-x64'], version)


def archive(version, target):
    targetDir = os.path.join(releaseDir, target)
    
    os.makedirs('{}/{}/'.format(tempDir,target), 0o777, True)
    print('Archiving ' + target)
    os.rename(targetDir + '/publish', targetDir + '/SlugChess')
    #//shutil.make_archive(rootDir + '..\\temp\\win-64\\SlugChess_'+version, '7zip', releaseDir+'\\win-64\\', 'publish')
    #shutil.make_archive(tempDir + '/'+ target + '/SlugChess_'+version, '7zip', 'publish' , releaseDir + '/'+ target +'/publish')
    make_archive('{}/{}/SlugChess'.format(releaseDir, target), '{}/{}/SlugChess_{}'.format(tempDir,target,version))
    os.rename(targetDir + '/SlugChess', targetDir + '/publish')


def make_archive(source, destination):
        base = os.path.basename(destination)

        archive_from = os.path.dirname(source)
        archive_to = os.path.basename(source.strip(os.sep))
        print(source, destination, archive_from, archive_to)
        shutil.make_archive(base, '7zip', archive_from, archive_to)
        shutil.move('%s.%s'%(base,'7z'), destination + '.7z')


def uploadArchive(targets, version):
    hostname = "ssh.spaceslug.no"
    username = "spaceslug.no"
    password = open(projectDir + '/../../spaceslug.no_SFTP_passord.txt', 'r').read()
    #since pysftp is shit on windows and can not find known_hosts from putty i must disable the security feature
    cnopts = pysftp.CnOpts()
    cnopts.hostkeys = None   
    
    with pysftp.Connection(host=hostname, username=username, password=password, cnopts=cnopts) as sftp:
        print("Connection with spaceslug.no succesfully stablished ... ")
        #directory_structure = sftp.listdir_attr()
        filename = 'SlugChess_{}.7z'.format(version)
        # Print data
        #for attr in directory_structure:
        #     print(attr.filename, attr)
        for target in targets:

            sftp.makedirs('./slugchess/releases/'+target, 774)
            sftp.makedirs('./slugchess/latest/'+target, 774)
            sftp.put('{}/{}/{}'.format(tempDir,target,filename), './slugchess/releases/{}/{}'.format(target, filename))
            sftp.put('{}/{}/{}'.format(tempDir,target,filename), './slugchess/latest/{}/{}'.format(target, 'SlugChess_latest.7z'))
            print('Uploaded '+target+' SlugChess_'+version)
     
# connection closed automatically at the end of the with-block