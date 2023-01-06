# 
# Copyright (C) 2023  Steffen Nuessle
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
# 

import argparse
import hashlib
import os
import subprocess
import sys

def main():
	parser = argparse.ArgumentParser(
		prog=sys.argv[0],
		description='Upload a file to Artifactory.',
		epilog=(
			'Additionally the environment variable "ARTIFACTORY_API_KEY" '
			'must be defined and contain a valid key which can be used '
			'as a value within a "X-JFrog-Art-Api" HTTP header.'
		)
	)
	parser.add_argument(
		'--repository-url',
		help='Base URL to an Artifactory repository.',
		metavar='URL',
		required=True,
		type=str
	)
	parser.add_argument(
		'--repository-path',
		help=(
			'The path inside the Artifactory repository where the new file '
			'shall be placed.'
		),
		metavar='PATH',
		required=False,
		type=str
	)
	parser.add_argument(
		'--file',
		help='The path to the file which shall get uploaded.',
		metavar='PATH',
		required=True,
		type=str
	)
	parser.add_argument(
		'--properties',
		help=(
			'Key-value pairs which get attached as properties to the '
			'uploaded file.'
		),
		metavar='KEY=VALUE',
		required=False,
		default=[],
		nargs='*',
		type=str
	)

	args = parser.parse_args()

	sha256 = hashlib.sha256()
	sha1 = hashlib.sha1()

	data = open(args.file, 'rb').read()
	sha256.update(data)
	sha1.update(data)

	sha256sum = sha256.hexdigest()
	sha1sum = sha1.hexdigest()

	key = os.environ.get('ARTIFACTORY_API_KEY', '')
	
	filename = os.path.basename(args.file)
	
	if len(args.properties) != 0:
		props = ';'.join([''] + args.properties)
	else:
		props = ''

	url = f'{args.repository_url}{props}/{args.repository_path}/{filename}'

	command = [
		'curl',
		'--silent',
		'--show-error',
		'--fail',
		'--write-out', '\n',
		'--request', 'PUT',
		'--header', f'X-JFrog-Art-Api:{key}',
		'--header', 'X-Checksum-Deploy:false',
		'--header', f'X-Checksum-Sha256:{sha256sum}',
		'--header', f'X-Checksum-Sha1:{sha1sum}',
		'--upload-file', args.file,
		url,
	]

	status = subprocess.run(command, check=False)

	sys.exit(status.returncode)

if __name__ == '__main__':
	main()

