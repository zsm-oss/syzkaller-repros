#!/bin/bash

set -e

rm -rf linux_kvm && mkdir linux_kvm
echo "Extracting KVM repros"
for i in $(egrep -r "kvm" ./linux | cut -d':' -f1 | sort | uniq); do
	cp $i ./linux_kvm
done

#echo "Processing repros"
#tempfile=$(mktemp)
#for i in ./linux_kvm/*; do
	#sed 's/initialize_tun();//g' $i > $tempfile
	#sed 's/initialize_netdevices();//g' $tempfile > $i
#done

echo "Building repros"
outdir=bin_kvm_x86_64
rm -rf $outdir && mkdir $outdir
for f in linux_kvm/*.c; do
	echo $f
	out="$outdir/`basename "$f" .c`"
	flags=""
	if grep "__NR_mmap2" $f; then
		flags="-m32"
	fi
	gcc "$f" -static -pthread $flags -o $out
done

echo "Creating archive"
zip -r bin_kvm_x86_64.zip bin_kvm_x86_64
mv bin_kvm_x86_64.zip bin_kvm_x86_64_$(date '+%Y%m%d').zip
