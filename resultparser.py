import sys
import argparse
import subprocess

def print_results(result_type, results):
    max_len = len(max(results, key=lambda result: len(result[0]))[0])
    print(result_type + ":")
    for result in results:
        print(("{0:" + str(max_len+1)+ "s} {1:.3f} {2:.3f}").format(result[0]+";", result[1][0], result[1][1]))

def parse_data(f, binary_path):
    lines = list(f)
    first_line = lines[0].strip().split(',')
    baseaddr, total_cycles = int(first_line[0], 16), 1
    data = [x.strip().split(',') for x in lines[1:]]
    dedup = {}
    address_to_convert = [[int(addr, 16) - baseaddr,(int(waitticks)/1000000,int(runticks)/1000000)] for [addr,waitticks,runticks] in data if int(waitticks) == 0 or int(runticks) == 0]
    # for info in address_to_convert:
    #     if info[0] in dedup:
    #         dedup[info[0]] = (dedup[info[0]][0] + info[1][0],dedup[info[0]][1] + info[1][1])
    #     else:
    #         dedup[info[0]] = info[1]
    args = [format(addr,'#04x') for addr,_ in address_to_convert]
    with subprocess.Popen((['addr2line', '-f', '-e', binary_path[0]] + args), stdout=subprocess.PIPE) as proc:
        output = str(proc.stdout.read(), "UTF-8").split("\n")[:-1]
    consolidated_output = []
    for i, item in enumerate(output):
        if i % 2 == 0:
            consolidated_output.append(item)
        else:
            consolidated_output[int(i/2)] += ':{}'.format(item)
    result = [[line,_] for line, [addr, _] in zip(consolidated_output,address_to_convert)]
    print_results("Total Time", sorted(result, key=lambda packed: packed[1][0]+packed[1][1], reverse=True))

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Parses result.csv from POSIX-lock-profiler')
    parser.add_argument('binary', nargs=1, type=str)
    parser.add_argument('csv', nargs='?', type=argparse.FileType('r'),
                        default=sys.stdin)
    args = parser.parse_args()
    parse_data(args.csv, args.binary)
