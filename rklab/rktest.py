#!/usr/bin/env python

import subprocess, random, sys, time, re
from optparse import OptionParser

THRES=20
VERBOSE=0

def print_diff(x, y):
	if x == y:
		return 
	print "Your program's output differs from that of instructor's program"
	print "Below is a comparison line by line. "
	print "Lines that differ start with < or > and the first character they diverge is marked in []"
	print "----Output of instructor's program----"
	s1_list = re.split('\n', x)
	s2_list = re.split('\n', y)
	for j in range(len(s1_list)):
		s1 = s1_list[j]
		if (j < len(s2_list)):
			s2 = s2_list[j]
			if s1 == s2:
				sys.stdout.write(s1)
			else:
				sys.stdout.write("< ")
				diverged = False
				for i in range(len(s1)):
					if (i < len(s2) and s1[i]== s2[i]):
						sys.stdout.write(s1[i])
					elif i <= len(s2):
						if (not diverged):
							sys.stdout.write("["+s1[i]+"]")
							diverged = True
						else:
							sys.stdout.write(s1[i])
					else:
						sys.stdout.write(s1[i])
				if (not diverged) and (len(s1) < len(s2)):
					sys.stdout.write("[]")
			sys.stdout.write("\n")
		else:
			print s1

	print "----Output of your program----"
	for j in range(len(s2_list)):
		s2 = s2_list[j]
		if (j < len(s1_list)):
			s1 = s1_list[j]
			if s1 == s2:
				sys.stdout.write(s2)
			else:
				sys.stdout.write("> ")
				diverged = False
				for i in range(len(s2)):
					if (i < len(s1) and s2[i]== s1[i]):
						sys.stdout.write(s2[i])
					elif i <= len(s1):
						if (not diverged):
							sys.stdout.write("["+s2[i]+"]")
							diverged = True
						else:
							sys.stdout.write(s2[i])
					else:
						sys.stdout.write(s2[i])
				if (not diverged) and (len(s2) < len(s1)):
					sys.stdout.write("[]")
			sys.stdout.write("\n")
		else:
			print s2

def get_rand_nums(size):
	s = []
	count = 0
	while count < size:
		s.append(random.randint(1, 100000))
    	count = count + 1
	return s

def get_rand_string(size):
	s = []
	slen = 0
	wlen = 0
	prev = ' '
	while slen < size:
		if ((random.random() < 0.1 or wlen > 14) and prev!=' '):
			prev = ' '
			s.append(prev)
			wlen = 0
		else: 
		 	prev = random.choice('abcdefghijklmnopqrstuvwxyz')
			s.append(prev)
			wlen += 1
		slen += 1
	return s

def get_denormalized(orig):
	s = []
	for i in (range(len(orig))):
		if (random.random() < 0.5):
			s.append(orig[i])
		else:
			s.append(orig[i].upper())
		if (orig[i] == ' '):
			s.append('\t')
			for j in range(3):
				if (random.random() < 0.5):
					s.append(' ')
	return s

def write_to_file(s, fname):
	f = open(fname,'w')
	f.write(''.join(s))
	f.close()

def test_command(prog="./rkmatch", args=[], printtime=False):

	cmd = []
	cmd.append(prog+"-answer")
	for i in range(len(args)):
		cmd.append(args[i])

	if VERBOSE:
		print "--- run command '", ' '.join(cmd), "'"
	t1 = time.time()
#s1 = subprocess.Popen(["./rkanswer", "-t", str(algo), "-k", str(k), "X", "Y"],stdout=subprocess.PIPE).communicate()[0]
	p1 = subprocess.Popen(cmd,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
	[s1,ss1] = p1.communicate()
	r1 = p1.wait()
	t2 = time.time()

	cmd = []
	cmd.append(prog)
	for i in range(len(args)):
		cmd.append(args[i])

	if VERBOSE:
		print "--- run command '", ' '.join(cmd), "'"
#p2 = subprocess.Popen(["./rkmatch", "-t", str(algo), "-k", str(k), "X", "Y"],stdout=subprocess.PIPE,stderr=subprocess.PIPE)
	p2 = subprocess.Popen(cmd,stdout=subprocess.PIPE)
	[s2,ss2] = p2.communicate()
	r2 = p2.wait()
	t3 = time.time()

	if (r2 != 0) :
		if (r2 == -11 or r2 == 11) :
			print prog, " did not terminate normally (segmentation fault)"
		else:
			print prog, " did not terminate normally (returncode=%d)" % r2
		sys.exit(1)

	if (s1!=s2):
		print_diff(s1, s2)
		sys.exit(1)
	else:
		if printtime:
			print "\tinstructor's program completed in %.2f ms yours completed in %.2f ms" % ((t2-t1) *1000, (t3-t2)*1000)

def test_normalization(fsize):
	xs = get_rand_string(fsize)	
	write_to_file(xs,'X')
	ys = get_denormalized(xs)
	write_to_file(ys,'Y')
	print "   test with file X (",len(xs), "bytes) and Y (",len(ys), "bytes), Y is a denormalized version of X"
	test_command(prog="./rkmatch", args=["-t","0","X","Y"])
	

def test_bloom(bsz,seed):
  	print "   'bloom_test", bsz, seed,"\'"
	test_command(prog="./bloom_test", args=[str(bsz),str(seed)])
   

def test_near_match(algo,fsize):
	xs = get_rand_string(fsize)
	write_to_file(xs,'X')
	xxs = get_rand_string(fsize)
	yys = get_denormalized(xs)
	ylen = len(yys)
	shift = len(yys)
	for sh in range(3):
		ys = list(yys)
		shift = shift // 2
		for i in (range(shift)):
			ys.append(' ')
		for i in (range(ylen-1,-1,-1)):
			ys[i+shift] = ys[i]
		for i in (range(0,shift)):
			ys[i] = ys[i+ylen]
		del ys[ylen:]
		write_to_file(ys,'Y')

		print "   test with file X (",len(xs), "bytes) and Y (",len(ys), "bytes), Y is X shifted by ", shift, " chars"
		test_command(prog="./rkmatch",args=["-t",str(algo), "-k", str(THRES), "X", "Y"])
		for i in (range(0,shift)):
			ys[i] = xxs[i]
		write_to_file(ys,'Y')
		print "   test with file X (",len(xs), "bytes) and Y (",len(ys), "bytes), the last ", shift, "chars of X and Y are identical"
		test_command(prog="./rkmatch",args=["-t",str(algo), "-k", str(THRES), "X", "Y"])

def test_near_miss(algo,fsize):
	xs = get_rand_string(fsize)
	write_to_file(xs,'X')
	yys = get_rand_string(fsize)
	for i in range(3):
		start_xs = (random.randint(0,len(xs))/THRES)*THRES
		cut = random.randint(0, len(yys)-1)
		ys = yys[:cut]
		ys += xs[start_xs:start_xs+THRES]
		ys += yys[cut:len(yys)-1]
#ys = get_denormalized(ys)
		write_to_file(ys,'Y')
		print "   test with file X (",len(xs), "bytes) and Y (",len(ys), "bytes), Y has a ", THRES, "char string copied from X"
		test_command(prog="./rkmatch",args=["-t",str(algo), "-k", str(THRES), "X", "Y"])

if __name__ == '__main__':

	parser = OptionParser()
	parser.add_option("-t", "--typeofalgo", help="which algorithm to use for matching")
	parser.add_option("-v", "--verbose", action="store_true", help="print verbose tester output for debugging")

	(options, args) = parser.parse_args()
	VERBOSE = options.verbose

	if (options.typeofalgo == "exact" or options.typeofalgo is None):
                print "Testing exact match..."
		for i in range(3):
			test_normalization((2<<i)*1000)
		print "Test of exact match passed"

	if (options.typeofalgo == "simple" or options.typeofalgo is None): 
		print "Testing simple approximate match..."
		test_near_match(1, 30000)
		test_near_miss(1,30000)
		print "Test of simple approximate match passed"
  
	if (options.typeofalgo == "rk" or options.typeofalgo is None): 
		print "Testing RK algorithm..."
		test_near_match(2, 30000)
		test_near_miss(2,30000)
		print "Test of RK passed"
 
	if (options.typeofalgo == "bloom" or options.typeofalgo is None): 
		print "Test Bloom filter..."
		for i in range(3):
			test_bloom(1024,i)
		for i in range(3):
			test_bloom(65536,i)
		print "Test bloom filter passed"

	if (options.typeofalgo == "rkbatch" or options.typeofalgo is None): 
		print "Test RKBATCH ...."
		test_near_match(3, 30000)
		test_near_miss(3,30000)
		print "Test RKBATCH passed"

