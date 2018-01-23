#!/usr/bin/python

# Python zipfile have problems with files encrypted with AES algorithms,
# so this program only works with encrypted files using ZipCrypto.
# The ZIP file used was created/encrypted with the following command:
# 	zip secrets.zip -r <content> -e

# Author: Victor C. Leal
#
# 15/10/2017

import sys
import os
import argparse
import zipfile

def checkZip(path):
	"""
	Verify argument type received for zip files (to use with argparse)
	Returns only the filename of the zip, if it exists and it is a zip
	"""
	pathZip, zipfilename = os.path.split(path)
	if pathZip:
		os.chdir(pathZip)
	if not zipfile.is_zipfile(zipfilename):
		raise argparse.ArgumentTypeError(
            'argument filename must be of type *.zip')
	return zipfilename

def main():
	"""
	Program that does a bruteforce dictionary attack to find the password of a password protected ZIP file
	"""

	cwd = os.getcwd()
	# Argument and command-line options parsing
	parser = argparse.ArgumentParser(description='Brute-force dictionary attack for a ZIP file.')
	parser.add_argument('-l', required=True, metavar='file', dest='dictionary',
	                    help='Specify dictionary file')
	parser.add_argument('-f', required=True, metavar='file', dest='zipfilename', type=checkZip,
	                    help='Specify ZIP file')
	args = parser.parse_args()
	zip_file = zipfile.ZipFile(args.zipfilename)
	pathZip = os.getcwd()
	os.chdir(cwd)
	password = None
	# Brute force through dictionary entries
	with open(args.dictionary, 'r') as f:
		os.chdir(pathZip)
		for line in f.readlines():
			password = line.strip('\n')
			# Test passwords
			try:
				zip_file.extractall(pwd=password)
				print 'The password is %s' % password
				break
			except:
				pass

if __name__ == '__main__':
	main()
