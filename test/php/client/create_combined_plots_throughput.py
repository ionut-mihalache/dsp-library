import sys
import pandas as pd
import matplotlib.pyplot as plt


def load_csv(path):
    df = pd.read_csv(path)

    df = df.sort_values("messages_number")

    return df


def plot_throughput(output_dir, dfs, labels):
    plt.figure(figsize=(8,5))

    markers = ["o","s","^"]
    linestyles = ["-","--",":"]

    for i,df in enumerate(dfs):
        plt.plot(
            df["messages_number"],
            df["throughput"],
            marker=markers[i],
            linestyle=linestyles[i],
            label=labels[i]
        )

    plt.xlabel("Number of Messages")
    plt.ylabel("Throughput (msg/s)")
    plt.grid(True, linestyle=":")
    plt.legend(frameon=False)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/throughput_vs_messages.pdf")
    plt.close()


def plot_latency(output_dir, dfs, labels):

    plt.figure(figsize=(8,5))

    markers = ["o","s","^"]
    linestyles = ["-","--",":"]

    for i,df in enumerate(dfs):
        plt.plot(
            df["messages_number"],
            df["avgLatency"],
            marker=markers[i],
            linestyle=linestyles[i],
            label=labels[i]
        )

    plt.xlabel("Number of Messages")
    plt.ylabel("Average Latency (ms)")
    plt.grid(True, linestyle=":")
    plt.legend(frameon=False)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/avg_latency_vs_messages.pdf")
    plt.close()


def plot_tail_latency(output_dir, dfs, labels):

    plt.figure(figsize=(8,5))

    markers = ["o","s","^"]
    linestyles = ["-","--",":"]

    for i,df in enumerate(dfs):
        plt.plot(
            df["messages_number"],
            df["maxLatency"],
            marker=markers[i],
            linestyle=linestyles[i],
            label=labels[i]
        )

    plt.xlabel("Number of Messages")
    plt.ylabel("Tail Latency (ms)")
    plt.yscale("log")

    plt.grid(True, linestyle=":")
    plt.legend(frameon=False)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/tail_latency_vs_messages.pdf")
    plt.close()


def plot_latency_vs_throughput(output_dir, dfs, labels):

    plt.figure(figsize=(6,5))

    markers = ["o","s","^"]

    for i,df in enumerate(dfs):
        plt.plot(
            df["throughput"],
            df["avgLatency"],
            marker=markers[i],
            linestyle="-",
            label=labels[i]
        )

    plt.xlabel("Throughput (msg/s)")
    plt.ylabel("Average Latency (ms)")
    plt.grid(True, linestyle=":")
    plt.legend(frameon=False)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/latency_vs_throughput.pdf")
    plt.close()


def plot_normalized_throughput(output_dir, dfs, labels, baseline_index=1):

    baseline = dfs[baseline_index]

    plt.figure(figsize=(8,5))

    markers = ["o","s","^"]
    linestyles = ["-","--",":"]

    for i,df in enumerate(dfs):

        rel = df["throughput"] / baseline["throughput"]

        plt.plot(
            df["messages_number"],
            rel,
            marker=markers[i],
            linestyle=linestyles[i],
            label=f"{labels[i]}"
        )

    plt.axhline(1, linewidth=1)

    plt.xlabel("Number of Messages")
    plt.ylabel("Relative Throughput (vs UDS)")
    plt.grid(True, linestyle=":")
    plt.legend(frameon=False)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/normalized_throughput.pdf")
    plt.close()


def main():

    if len(sys.argv) < 4:
        print("Usage:")
        print("python create_combined_plot_throughput.py output_dir csv_aqua csv_uds [csv_zmq]")
        return

    output_dir = sys.argv[1]

    csvs = sys.argv[2:]

    labels = ["AQUA","UDS","ZeroMQ"]

    dfs = [load_csv(p) for p in csvs]

    labels = labels[:len(dfs)]

    plot_throughput(output_dir, dfs, labels)
    plot_latency(output_dir, dfs, labels)
    plot_tail_latency(output_dir, dfs, labels)
    plot_latency_vs_throughput(output_dir, dfs, labels)

    if len(dfs) >= 2:
        plot_normalized_throughput(output_dir, dfs, labels, baseline_index=1)


if __name__ == "__main__":
    main()