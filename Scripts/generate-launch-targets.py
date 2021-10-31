#!/usr/bin/env python3

import os
import json
import argparse

isWindows = (os.name == 'nt')

parser = argparse.ArgumentParser()

parser.add_argument(
    '--name',
    required=True,
    help='Name of the launch target to generate.'
)

parser.add_argument(
    '--executable',
    required=True,
    help='Path to executable to run, relative to binary-dir.'
)

parser.add_argument(
    '--binary-dir',
    required=True,
    help='CMake\'s Binary directory, will be used with executable.'
)

parser.add_argument(
    '--working-dir',
    required=True,
    help='Directory to run the executable from.'
)

parser.add_argument(
    '--asset-path',
    required=True,
    help='Semicolon-delimited list of asset paths, will be used in ASSET_PATH.'
)

parser.add_argument(
    '--runtime-path',
    required=True,
    help='Semicolon-delimted list of runtime paths, will be used in PATH or LD_LIBRARY_PATH.'
)

args = parser.parse_args()

name       = args.name
executable = args.executable
binaryDir  = args.binary_dir
workingDir = args.working_dir
rootDir    = os.getcwd()

# Replace with OS-specific multiple path separator (';' for PATH, ':' for LD_LIBRARY_PATH)
assetPath   = args.asset_path.replace(';', os.pathsep)
runtimePath = args.runtime_path.replace(';', os.pathsep)

def add_or_update_config(data, configurations):
    found = False
    for i in range(0, len(configurations)):
        if configurations[i]['name'] == data['name']:
            configurations[i] = data
            found = True
            break
    
    if not found:
        configurations.append(data)

if os.path.isdir('.vscode'):
    filename = '.vscode/launch.json'

    launch = {}
    try:
        file = open(filename, 'r')
        launch = json.load(file)
    except:
        pass

    if 'version' not in launch:
        launch['version'] = '0.2.0'

    if 'configurations' not in launch:
        launch['configurations'] = []

    data = {
        'name': name,
        'type': 'cppdbg',
        'request': 'launch',
        'program': binaryDir + '/' + executable,
        'args': [],
        'cwd': workingDir,
        'environment': [
            {
                'name': 'RYME_ASSET_PATH',
                'value': assetPath
            }
        ]
    }
    
    if isWindows:
        data['type'] = 'cppvsdbg'
        data['environment'].append({
            'name': 'PATH',
            'value': '${env:PATH};' + runtimePath
        })
        data['console'] = 'internalConsole'
        data['logging']= {
            'moduleLoad': False
        }
    else:
        data['environment'].append({
            'name': 'LD_LIBRARY_PATH',
            'value': runtimePath
        })
        data['externalConsole'] = False

    add_or_update_config(data, launch['configurations'])

    file = open(filename, 'w')
    json.dump(launch, file, indent=4)

if isWindows and os.path.isdir('.vs'):
    filename = '.vs/launch.vs.json'

    launch = {}
    try:
        file = open(filename, 'r')
        launch = json.load(file)
    except:
        pass

    if 'version' not in launch:
        launch['version'] = '0.2.1'

    if 'configurations' not in launch:
        launch['configurations'] = []

    data = {
        'name': name,
        'type': 'default',
        'project': 'CMakeLists.txt',
        'args': [],
        'projectTarget': '{} ({})'.format(os.path.basename(executable), executable.replace('/', '\\')),
        'cwd': workingDir,
        'env': {
            'PATH': '${env.PATH};' + runtimePath,
            'RYME_ASSET_PATH': assetPath,
        },
    }

    add_or_update_config(data, launch['configurations'])

    file = open(filename, 'w')
    json.dump(launch, file, indent=4)
