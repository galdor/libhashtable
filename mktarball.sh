#!/bin/sh -e

files=$(git ls-files)
version=$(cat version)
dirname=libhashtable-$version
tarball=$dirname.tgz

git archive --format=tar --prefix=$dirname/ HEAD | gzip >| $tarball

tar tzf $tarball
echo
openssl sha1 $tarball
