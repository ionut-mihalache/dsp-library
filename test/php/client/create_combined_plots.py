import sys
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def load_and_prepare(csv_path, time_columns):
    df = pd.read_csv(csv_path)

    # Convert microseconds → ms
    df[time_columns] = df[time_columns] / 1000.0

    # Group by number of clients
    df_grouped = df.groupby("clients_nr")[time_columns].mean().reset_index()
    df_grouped = df_grouped.sort_values("clients_nr")

    return df_grouped


def create_combined_client_exec_time_plot2(output_dir, csv_a, csv_b, label_a="AQUA", label_b="ZeroMQ"):
    time_columns = ["connect", "call", "return", "disconnect"]

    # Load and prepare both files
    df_a = load_and_prepare(csv_a, time_columns)
    df_b = load_and_prepare(csv_b, time_columns)

    # Line styles and markers
    line_styles = ["-", "--"]
    markers = ["o", "s"]
    colors = ["black", "gray"]

    # Create figure
    fig, axs = plt.subplots(len(time_columns), 1, figsize=(10, 10), sharex=True)

    for i, col in enumerate(time_columns):
        y_a = df_a[col] / df_b[col]
        y_b = df_b[col] / df_b[col]

        axs[i].plot(
            df_a["clients_nr"], y_a,
            linestyle=line_styles[0],
            marker=markers[0],
            color=colors[0],
            label=f"{col.capitalize()} ({label_a})",
        )

        axs[i].plot(
            df_b["clients_nr"], y_b,
            linestyle=line_styles[1],
            marker=markers[1],
            color=colors[1],
            label=f"{col.capitalize()} ({label_b})",
        )

        # axs[i].set_ylabel("Relative Time (× first value)")
        axs[i].set_ylabel("Relative to UDS")
        axs[i].grid(True, linestyle=":")
        axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    plt.suptitle("Client Metrics vs Number of Clients (Two Datasets, Averaged, ms)", fontsize=14)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/combined_client_metrics_time_per_clients2.pdf", format="pdf")

def create_combined_client_exec_time_plot2_bar(output_dir, csv_a, csv_b, label_a="AQUA", label_b="ZeroMQ"):
    time_columns = ["connect", "call", "return", "disconnect"]

    df_a = load_and_prepare(csv_a, time_columns)
    df_b = load_and_prepare(csv_b, time_columns)

    clients = df_a["clients_nr"].to_numpy()
    x = np.arange(len(clients))
    width = 0.35

    fig, axs = plt.subplots(len(time_columns), 1, figsize=(10, 10), sharex=True)

    for i, col in enumerate(time_columns):
        y_a = (df_a[col] / df_b[col]).to_numpy()
        y_b = np.ones_like(y_a)

        axs[i].bar(x - width/2, y_a, width, label=f"{col.capitalize()} ({label_a})")
        axs[i].bar(x + width/2, y_b, width, label=f"{col.capitalize()} ({label_b})")

        axs[i].axhline(1, linewidth=1.2)  # Baseline (UDS)
        axs[i].set_ylabel("Relative to UDS")
        axs[i].grid(True, linestyle=":", axis="y")

        # Dynamic y-limits centered around baseline
        all_vals = np.concatenate([y_a, y_b])
        ymin = min(0.9, all_vals.min() * 0.95)
        ymax = all_vals.max() * 1.1
        axs[i].set_ylim(ymin, ymax)

        axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    axs[-1].set_xticks(x)
    axs[-1].set_xticklabels(clients)

    plt.suptitle("Client Metrics vs Number of Clients (Two Datasets, Averaged, ms)", fontsize=14)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/combined_client_metrics_time_per_clients2_bar.pdf", format="pdf")

def create_combined_client_exec_time_plot3(output_dir, csv_a, csv_b, csv_c, label_a="AQUA", label_b="ZeroMQ", label_c="UDS"):
    time_columns = ["connect", "call", "return", "disconnect"]

    # Load and prepare both files
    df_a = load_and_prepare(csv_a, time_columns)
    df_b = load_and_prepare(csv_b, time_columns)
    df_c = load_and_prepare(csv_c, time_columns)

    # Line styles and markers
    line_styles = ["-", "--", ":"]
    markers = ["o", "s", "v"]
    colors = ["deepskyblue", "navy", "blue"]

    # Create figure
    fig, axs = plt.subplots(len(time_columns), 1, figsize=(10, 10), sharex=True)

    for i, col in enumerate(time_columns):
        y_a = df_a[col] / df_b[col]
        y_b = df_b[col] / df_b[col]
        y_c = df_c[col] / df_b[col]

        # Plot dataset A
        axs[i].plot(
            df_a["clients_nr"],
            y_a,
            linestyle=line_styles[0],
            marker=markers[0],
            color=colors[0],
            label=f"{col.capitalize()} ({label_a})",
        )

        # Plot dataset B
        axs[i].plot(
            df_b["clients_nr"],
            y_b,
            linestyle=line_styles[1],
            marker=markers[1],
            color=colors[1],
            label=f"{col.capitalize()} ({label_b})",
        )

        # Plot dataset C
        axs[i].plot(
            df_c["clients_nr"],
            y_c,
            linestyle=line_styles[2],
            marker=markers[2],
            color=colors[2],
            label=f"{col.capitalize()} ({label_c})",
        )

        # axs[i].set_yscale("log", nonpositive="mask")
        axs[i].set_ylabel("Relative to UDS")
        axs[i].grid(True)
        axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    plt.suptitle("Client Metrics vs Number of Clients (Three Datasets, Averaged, ms)", fontsize=14)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/combined_client_metrics_time_per_clients3.pdf", format="pdf")

def create_combined_client_exec_time_plot3_bar(output_dir, csv_a, csv_b, csv_c, label_a="AQUA", label_b="ZeroMQ", label_c="UDS"):
    time_columns = ["connect", "call", "return", "disconnect"]

    # Load and prepare both files
    df_a = load_and_prepare(csv_a, time_columns)
    df_b = load_and_prepare(csv_b, time_columns)
    df_c = load_and_prepare(csv_c, time_columns)

    clients = df_a["clients_nr"].values
    x = np.arange(len(clients))
    width=0.35

    # Line styles and markers
    # line_styles = ["-", "--", ":"]
    # markers = ["o", "s", "v"]
    # colors = ["deepskyblue", "navy", "blue"]

    # Create figure
    fig, axs = plt.subplots(len(time_columns), 1, figsize=(10, 10), sharex=True)

    for i, col in enumerate(time_columns):
        # y_a = df_a[col] / df_b[col]
        # y_b = df_b[col] / df_b[col]
        # y_c = df_c[col] / df_b[col]
        y_a = (df_a[col] / df_b[col]).to_numpy()
        y_b = np.ones_like(y_a)
        y_c = (df_c[col] / df_b[col]).to_numpy()

        axs[i].bar(x - width, y_a, width, label=f"{col.capitalize()} ({label_a})")
        axs[i].bar(x,         y_b, width, label=f"{col.capitalize()} ({label_b})")
        axs[i].bar(x + width, y_c, width, label=f"{col.capitalize()} ({label_c})")

        axs[i].axhline(1, linewidth=1.2)
        axs[i].set_ylabel("Relative to UDS")
        axs[i].grid(True, linestyle=":", axis="y")

        # Dynamic y-limits
        all_vals = np.concatenate([y_a, y_b, y_c])
        ymin = min(0.9, all_vals.min() * 0.95)
        ymax = all_vals.max() * 1.1
        axs[i].set_ylim(ymin, ymax)

        axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    axs[-1].set_xticks(x)
    axs[-1].set_xticklabels(clients)

    plt.suptitle("Client Metrics vs Number of Clients (Three Datasets, Averaged, ms)", fontsize=14)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/combined_client_metrics_time_per_clients3_bar.pdf", format="pdf")

if __name__ == "__main__":
    # Args:
    # sys.argv[1] = output directory
    # sys.argv[2] = CSV A
    # sys.argv[3] = CSV B
    # sys.argv[4] = CSV C

    if len(sys.argv) == 4:
        create_combined_client_exec_time_plot2(sys.argv[1], sys.argv[2], sys.argv[3], "AQUA", "UDS")
        create_combined_client_exec_time_plot2_bar(sys.argv[1], sys.argv[2], sys.argv[3], "AQUA", "UDS")
    elif len(sys.argv) == 5:
        create_combined_client_exec_time_plot3(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], "AQUA", "UDS", "ZeroMQ")
        create_combined_client_exec_time_plot3_bar(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4], "AQUA", "UDS", "ZeroMQ")
