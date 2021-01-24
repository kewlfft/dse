#!/bin/bash
rm file1.enc file1.dec
../dse keyfile e file1 file1.enc
../dse keyfile d file1.enc file1.dec
diff file1.dec file1
if ($status); then
    echo "OK"
fi