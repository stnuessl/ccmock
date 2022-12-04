#
# The MIT License (MIT)
#
# Copyright (c) 2022  Steffen Nuessle
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
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

