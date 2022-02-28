from os import path
import csv


def main():
    with open(path.join(path.dirname(path.realpath(__file__)), "../../../syscall_num.csv"), "r") as ifile:
        csv_reader = csv.reader(ifile, delimiter=',')
        with open(path.join(path.dirname(path.realpath(__file__)), "syscall.S"), "w") as ofile:
            line_count = 0
            for row in csv_reader:
                if line_count == 0:
                    line_count += 1
                else:
                    line_count += 1
                    ofile.write(f".global tb_{row[0]}\n")
                    ofile.write(f"tb_{row[0]}:\n")
                    ofile.write(f"\tli a7, {row[1]}\n")
                    ofile.write(f"\tecall\n")
                    ofile.write(f"\tret\n\n")


if __name__ == "__main__":
    main()
