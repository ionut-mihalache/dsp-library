import sys
import pandas as pd
import matplotlib.pyplot as plt

def load_and_prepare(csv_path, time_columns):
    df = pd.read_csv(csv_path)

    # Convert microseconds → ms
    df[time_columns] = df[time_columns] / 1000.0

    # Group by number of clients
    df_grouped = df.groupby("clients_nr")[time_columns].mean().reset_index()
    df_grouped = df_grouped.sort_values("clients_nr")

    return df_grouped


def create_combined_client_exec_time_plot(output_dir, csv_a, csv_b, label_a="AQUA", label_b="ZeroMQ"):
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
        # Plot dataset A
        axs[i].plot(
            df_a["clients_nr"],
            df_a[col],
            linestyle=line_styles[0],
            marker=markers[0],
            color=colors[0],
            label=f"{col.capitalize()} ({label_a})",
        )

        # Plot dataset B
        axs[i].plot(
            df_b["clients_nr"],
            df_b[col],
            linestyle=line_styles[1],
            marker=markers[1],
            color=colors[1],
            label=f"{col.capitalize()} ({label_b})",
        )

        axs[i].set_ylabel("Time (ms)")
        axs[i].grid(True)
        axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    plt.suptitle("Client Metrics vs Number of Clients (Two Datasets, Averaged, ms)", fontsize=14)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/combined_client_metrics_time_per_clients_bw.pdf", format="pdf")


if __name__ == "__main__":
    # Args:
    # sys.argv[1] = output directory
    # sys.argv[2] = CSV A
    # sys.argv[3] = CSV B
    create_combined_client_exec_time_plot(sys.argv[1], sys.argv[2], sys.argv[3])