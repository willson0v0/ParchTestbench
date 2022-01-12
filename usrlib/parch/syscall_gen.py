from os import path

syscall_num = {
	'write'	: '0',
	'read'	: '1',
	'open'	: '2',
	'openat': '3',
	'fork'	: '4',
	'exec'	: '5',
	'exit'	: '6',
}

def main():
	with open(path.join(path.dirname(path.realpath(__file__)), 'syscall.S'), 'w') as ofile:
		for name, num in syscall_num.items():
			ofile.write(f".global tb_{name}\n")
			ofile.write(f"tb_{name}:\n")
			ofile.write(f"\tli a7, {num}\n")
			ofile.write(f"\tecall\n")
			ofile.write(f"\tret\n\n")

if __name__ == "__main__":
	main()