b = ''
with open("/home/abhigyan/project/configuration", "r") as f:
	b = f.readlines()
b = ''.join(b)
with open("/proc/mouseges_proc_file", "w") as f:
	f.write(b)
exit()
