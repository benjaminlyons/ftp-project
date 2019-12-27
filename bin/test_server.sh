#!/bin/sh

test_ls(){
	cat <<EOF
ls
quit
EOF
}

test_pwd(){
	cat << EOF
pwd
quit
EOF
}

test_get(){
	cat << EOF
get testfiles-$R
quit
EOF
}

test_delete(){
	cat << EOF
delete delete-test-$R
quit
EOF
}

test_put(){
	cat << EOF
put put-test-$R
quit
EOF
}

test_cd(){
	cat << EOF
cd testdir-$R
get dictionary
cd ..
get testfiles-$R
quit
EOF
}

test_mkdir(){
	cat << EOF
mkdir mkdir-test-$R
quit
EOF
}


# initialize variables
RANDOM=$$
PORT=$((9000 + $RANDOM % 1000))
R=$(($RANDOM % 10))
HOME=$(pwd)

# create temporary directory for testing
SCRATCH_SERVER=$(mktemp -d -t server-test-XXXXXXXX)
SCRATCH_CLIENT=$(mktemp -d -t client-test-XXXXXXXX) 
# add random stuff to it
for num in $(seq 0 9)
do
	mkdir -p $SCRATCH_SERVER/testdir-$num
	touch $SCRATCH_SERVER/testfiles-$num
	seq 0 $num > $SCRATCH_SERVER/testfiles-$num
done
cp /usr/share/dict/words $SCRATCH_SERVER/testdir-$R/dictionary


# start the server
echo "Starting the server"
./bin/server $PORT $SCRATCH_SERVER 2> .log & 

sleep 2

head -n 1 .log

# move the client to a special location
cp ./bin/client $SCRATCH_CLIENT
cd $SCRATCH_CLIENT

# testing ls
echo -n "Testing ls..."
LS_OUT=$(test_ls | $SCRATCH_CLIENT/client localhost $PORT | grep -v 'ftp>')
if [ $(echo $LS_OUT | wc -c) = 260 ] && 
   [ $(echo $LS_OUT | grep "testdir-$R" | wc -l) = 1  ]; then
	echo "Success"
else
	echo "Failure"
fi

# testing pwd
echo -n "Testing pwd..."
if [ $(test_pwd | $SCRATCH_CLIENT/client localhost $PORT | grep -v 'ftp>') = $SCRATCH_SERVER ]; then
	echo "Success"
else
	echo "Failure"
fi

# testing get
echo -n "Testing get..."
if [ $(test_get | $SCRATCH_CLIENT/client localhost $PORT 2>&1 | grep -Eo "(File size: [0-9]+)" | cut -d ' ' -f 3) -eq $(cat $SCRATCH_SERVER/testfiles-$R | wc -c) ] &&
   diff -u $SCRATCH_CLIENT/testfiles-$R $SCRATCH_SERVER/testfiles-$R ; then
	echo "Success"
else
	echo "Failure"
fi

# NOTE: at some point i need to get a success response from the server so i don't have to sleep
# testing delete
echo -n "Testing delete..."
seq 1 10 > $SCRATCH_SERVER/delete-test-$R
test_delete | $SCRATCH_CLIENT/client localhost $PORT 2>&1 > /dev/null
sleep 1
if [ ! -e $SCRATCH_SERVER/delete-test-$R ]; then
	echo "Success"
else
	echo "Failure"
fi

# test the put function
echo -n "Testing put..."
seq 1 10 > $SCRATCH_CLIENT/put-test-$R
test_put | $SCRATCH_CLIENT/client localhost $PORT 2>&1 | echo > /dev/null
sleep 1
if diff $SCRATCH_CLIENT/put-test-$R $SCRATCH_SERVER/put-test-$R ; then
	echo "Success"
else
	echo "Failure"
fi

# test the cd function
echo -n "Testing cd..."
if [ -e $SCRATCH_CLIENT/testfiles-$R ] ; then
	rm $SCRATCH_CLIENT/testfiles-$R
fi

test_cd | $SCRATCH_CLIENT/client localhost $PORT > /dev/null 2>&1
if [ -e $SCRATCH_CLIENT/dictionary ] && [ -e $SCRATCH_CLIENT/testfiles-$R ]; then
	echo "Success"
else
	echo "Failure"
fi

# test mkdir
echo -n "Testing mkdir..."
test_mkdir | $SCRATCH_CLIENT/client localhost $PORT > /dev/null 2>&1
sleep 1
if [ -d $SCRATCH_SERVER/mkdir-test-$R ]; then
	echo "Success"
else
	echo "Failure"
fi

# cleanup
cd $HOME
rm -fr $SCRATCH_SERVER $SCRATCH_CLIENT
ps ux | grep ./bin/server | grep -v grep | awk '{ print $2 }' | xargs kill
