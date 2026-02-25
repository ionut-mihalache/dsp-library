import sys
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import LogLocator, LogFormatter
# from scipy.stats import mstats
# from scipy.stats import trim_mean


def load_and_prepare(csv_path, time_columns):
    df = pd.read_csv(csv_path)

    # Convert microseconds → ms
    df[time_columns] = df[time_columns] / 1000.0

    # def remove_outliers(group, cols):
    #     Q1 = group[cols].quantile(0.25)
    #     Q3 = group[cols].quantile(0.75)
    #     IQR = Q3 - Q1

    #     mask = ~((group[cols] < (Q1 - 1.5 * IQR)) |
    #             (group[cols] > (Q3 + 1.5 * IQR))).any(axis=1)

    #     return group[mask]

    # df = (
    #     df.groupby("clients_nr", group_keys=False)
    #     .apply(lambda g: remove_outliers(g, time_columns))
    # )
    # for col in time_columns:
    #     Q1 = df[col].quantile(0.25)
    #     Q3 = df[col].quantile(0.75)
    #     IQR = Q3 - Q1
    #     lower = Q1 - 1.5 * IQR
    #     upper = Q3 + 1.5 * IQR
    #     df = df[(df[col] >= lower) & (df[col] <= upper)]

    # Group by number of clients
    df_grouped = df.groupby("clients_nr")[time_columns].mean().reset_index()
    df_grouped = df_grouped.sort_values("clients_nr")

    # print(df_grouped)

    # for col in time_columns:
    #     q1 = df_grouped[col].quantile(0.25)
    #     q3 = df_grouped[col].quantile(0.75)
    #     iqr = q3 - q1
    #     print(f"\n{col}")
    #     print("IQR:", iqr)
    #     print("Lower bound:", q1 - 1.5 * iqr)
    #     print("Upper bound:", q3 + 1.5 * iqr)
    #     print("Max value:", df_grouped[col].max())

    # print(df_grouped)

    return df_grouped

def load_and_prepare2(csv_path, time_columns, remove_outliers=False):
    df = pd.read_csv(csv_path)

    # Convert microseconds → ms
    df[time_columns] = df[time_columns] / 1000.0

    if remove_outliers:
        for col in time_columns:
            q1 = df[col].quantile(0.25)
            q3 = df[col].quantile(0.75)
            iqr = q3 - q1
            lower = q1 - 1.5 * iqr
            upper = q3 + 1.5 * iqr

            df = df[(df[col] >= lower) & (df[col] <= upper)]

    # Group by number of clients
    df_grouped = (
        df.groupby("clients_nr")[time_columns]
          .agg(["median", lambda x: x.quantile(0.95)])
    )

    # Rename columns cleanly
    df_grouped.columns = [
        f"{col}_{stat if stat != '<lambda>' else 'p95'}"
        for col, stat in df_grouped.columns
    ]

    df_grouped = df_grouped.reset_index().sort_values("clients_nr")

    return df_grouped

def load_and_prepare3(csv_path, time_columns, remove_outliers=True, iqr_factor=1.5):
    """
    Load CSV, convert microseconds → ms, optionally remove outliers per group, and return
    median per clients_nr.
    """
    df = pd.read_csv(csv_path)

    # Convert microseconds → ms
    df[time_columns] = df[time_columns] / 1000.0

    # if remove_outliers:
    #     # Eliminare outlier per grup
    #     def filter_group_outliers(group):
    #         for col in time_columns:
    #             q1 = group[col].quantile(0.25)
    #             q3 = group[col].quantile(0.75)
    #             iqr = q3 - q1
    #             lower = q1 - iqr_factor * iqr
    #             upper = q3 + iqr_factor * iqr
    #             # Filtrare valori extreme
    #             group = group[(group[col] >= lower) & (group[col] <= upper)]
    #         return group

    #     df = df.groupby("clients_nr", group_keys=False, as_index=False).apply(filter_group_outliers)

    for col in time_columns:
        max_val = df[col].quantile(0.99)
        # print(max_val)
        df[col] = df[col].clip(upper=max_val)

    # def stabilize(group):
    #     for col in time_columns:
    #         lower = group[col].quantile(0.05)
    #         upper = group[col].quantile(0.95)
    #         group[col] = group[col].clip(upper=upper)
    #     return group

    # df = s

    # Agregare mediană per număr de clienți
    df_grouped = df.groupby("clients_nr", as_index=False)[time_columns].mean()
    # df_grouped = df.groupby("clients_nr")[time_columns].agg(
    #     lambda x: trim_mean(x, 0.1)  # elimină 10% valori extreme din ambele capete
    # ).reset_index()

    # Optional: afișare statistică pentru verificare
    # for col in time_columns:
    #     q1 = df_grouped[col].quantile(0.25)
    #     q3 = df_grouped[col].quantile(0.75)
    #     iqr = q3 - q1
        # print(f"\n{col} | IQR: {iqr:.6f} | Min: {df_grouped[col].min():.6f} | Max: {df_grouped[col].max():.6f}")

    return df_grouped.sort_values("clients_nr")

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
        # if col == "connect" or col == "disconnect":
        axs[i].set_yscale("log")
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
    width = 0.28

    fig, axs = plt.subplots(len(time_columns), 1, figsize=(10, 10), sharex=True)

    for i, col in enumerate(time_columns):
        y_a = (df_a[col] / df_b[col]).to_numpy()
        y_b = np.ones_like(y_a)

        # axs[i].bar(x - width/1.5, y_a, width, label=f"{col.capitalize()} ({label_a})", edgecolor="black", linewidth=0.8, hatch="///", facecolor="white")
        # axs[i].bar(x + width/1.5, y_b, width, label=f"{col.capitalize()} ({label_b})",edgecolor="black", linewidth=0.8, hatch="xxx", facecolor="lightgray")
        axs[i].bar(
            x - width/1.5,
            y_a,
            width,
            label=f"{label_a}",
            edgecolor="black",
            linewidth=0.8,
            hatch="///",
            facecolor="white"
        )
        axs[i].bar(
            x + width/1.5,
            y_b,
            width,
            label=f"{label_b}",
            edgecolor="black",
            linewidth=0.8,
            hatch="xxx",
            facecolor="lightgray"
        )

        # if col == "connect" or col == "disconnect":
        axs[i].set_yscale("log")

        axs[i].axhline(1, linewidth=1.2)  # Baseline (UDS)
        axs[i].set_ylabel("Relative to UDS")
        # axs[i].set_ylabel(col.capitalize())
        axs[i].grid(True, linestyle=":", axis="y")

        # Dynamic y-limits centered around baseline
        all_vals = np.concatenate([y_a, y_b])
        ymin = min(0.9, all_vals.min() * 0.95)
        ymax = all_vals.max() * 1.5
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

        # if col == "connect" or col == "disconnect":
        axs[i].set_yscale("log")

        axs[i].yaxis.set_major_locator(LogLocator(base=10))
        axs[i].yaxis.set_minor_locator(LogLocator(base=10.0, subs=np.arange(2, 10) * 0.1))

        axs[i].yaxis.set_minor_formatter(LogFormatter(base=10.0))

        # axs[i].grid(True, which="major", linestyle="-", alpha=0.6)
        # axs[i].grid(True, which="minor", linestyle=":", alpha=0.3)

        axs[i].set_ylabel("Relative to UDS")
        axs[i].grid(True)
        axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    plt.suptitle("Client Metrics vs Number of Clients (Three Datasets, Averaged, ms)", fontsize=14)

    plt.tight_layout()
    plt.savefig(f"{output_dir}/combined_client_metrics_time_per_clients3.pdf", format="pdf")

def create_combined_client_exec_time_plot3_bar(output_dir, csv_a, csv_b, csv_c, label_a="AQUA", label_b="ZeroMQ", label_c="UDS"):
    time_columns = ["connect", "call", "return", "disconnect"]

    df_a = load_and_prepare(csv_a, time_columns)
    df_b = load_and_prepare(csv_b, time_columns)
    df_c = load_and_prepare(csv_c, time_columns)

    clients = df_a["clients_nr"].to_numpy()
    x = np.arange(len(clients))
    width = 0.2

    fig, axs = plt.subplots(len(time_columns), 1, figsize=(10, 10), sharex=True)

    for i, col in enumerate(time_columns):
        y_a = (df_a[col] / df_b[col]).to_numpy()
        y_b = np.ones_like(y_a)
        # y_b = (df_b[col] / df_b[col]).to_numpy()
        y_c = (df_c[col] / df_b[col]).to_numpy()

        # axs[i].bar(x - width*1.05, y_a, width, label=f"{col.capitalize()} ({label_a})", hatch="|||")
        # axs[i].bar(x,             y_b, width, label=f"{col.capitalize()} ({label_b})", hatch="xxx")
        # axs[i].bar(x + width*1.05, y_c, width, label=f"{col.capitalize()} ({label_c})", hatch="...")
        axs[i].bar(
            x - width * 1.05,
            y_a,
            width,
            label=f"{label_a}",
            hatch="|||"
        )
        axs[i].bar(
            x,
            y_b,
            width,
            label=f"{label_b}",
            hatch="xxx"
        )
        axs[i].bar(
            x + width * 1.05,
            y_c,
            width,
            label=f"{label_c}",
            hatch="..."
        )

        axs[i].axhline(1, linewidth=1.1)
        # axs[i].set_ylabel("Relative to UDS")
        axs[i].set_ylabel(col.capitalize())

        all_vals = np.concatenate([y_a, y_b, y_c])
        all_vals = all_vals[np.isfinite(all_vals)]

        min_val = all_vals.min()
        max_val = all_vals.max()

        ratio = max_val / max(min_val, 1e-9)

        # dacă diferența e foarte mare -> log scale
        if ratio > 25:
            axs[i].set_yscale("log")

        # if col == "connect" or col == "disconnect":
        # if y_a // y_b > 10 or y_c // y_b > 10:
        #     axs[i].set_yscale("log")

        # axs[i].yaxis.set_major_locator(LogLocator(base=10))
        # axs[i].yaxis.set_minor_locator(LogLocator(base=10, subs='auto'))

        axs[i].grid(True, linestyle=":", axis="y")

        handles, labels = axs[0].get_legend_handles_labels()
        fig.legend(
            handles,
            labels,
            loc="upper center",
            ncol=3,
            frameon=False,
            bbox_to_anchor=(0.5, 0.96)   # ← mai jos decât înainte
        )

        # all_vals = np.concatenate([y_a, y_b, y_c])
        # ymin = min(0.9, all_vals.min() * 0.95)
        # ymax = all_vals.max() * 2.5
        # axs[i].set_ylim(ymin, ymax)

        # axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    axs[-1].set_xticks(x)
    axs[-1].set_xticklabels(clients)

    # Ridicăm titlul puțin mai sus
    fig.suptitle(
        "Client Metrics vs Number of Clients (Three Datasets, Averaged, ms)",
        fontsize=14,
        y=0.995
    )

    plt.tight_layout(rect=[0, 0, 1, 0.95])
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
