import matplotlib.pyplot as plt
import sys

def main():
    filename = sys.argv[1]
    with open(filename) as f:
        plot_data = f.read().splitlines()
        plt.suptitle(plot_data[0])
        plt.xlabel(plot_data[1])
        plt.ylabel(plot_data[2])
        n = int(plot_data[3])
        for i in range(n):
            data_idx = 4+i*3
            plt.plot(
                list(map(float, plot_data[data_idx+1].split())),
                list(map(float, plot_data[data_idx+2].split())),
                label=plot_data[data_idx]
            )
        plt.legend()
        plt.show()

if __name__ == "__main__":
    main()