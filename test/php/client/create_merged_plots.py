import sys
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

TIME_COLUMNS = ["connect", "call", "return", "disconnect"]
# TIME_COLUMNS = ["connect", "call", "return", "disconnect"]


def load_and_prepare(csv_path, time_columns):
    df = pd.read_csv(csv_path)

    # Convert microseconds -> ms
    df[time_columns] = df[time_columns] / 1000.0

    # Group by number of clients
    df_grouped = df.groupby("clients_nr")[time_columns].mean().reset_index()
    df_grouped = df_grouped.sort_values("clients_nr")

    return df_grouped


def create_bar_plot2(
    payloads_csv_paths: dict[tuple[str]],
    labels: tuple[str] = ("AQUA", "UDS"),
    outdir: str = ".",
):
    label_a, label_b = labels

    payloads_nr = len(payloads_csv_paths)
    metrics_nr = len(TIME_COLUMNS)

    fig, axs = plt.subplots(payloads_nr, metrics_nr, figsize=(16, 12), sharex="col")

    width = 0.2

    for row, (payload_label, csv_paths) in enumerate(payloads_csv_paths.items()):
        csv_a, csv_b = csv_paths

        df_a = load_and_prepare(csv_a, TIME_COLUMNS)
        df_b = load_and_prepare(csv_b, TIME_COLUMNS)

        clients = df_a["clients_nr"].to_numpy()
        x = np.arange(len(clients))

        for col_idx, col in enumerate(TIME_COLUMNS):
            ax = axs[row, col_idx]

            y_a = (df_a[col] / df_b[col]).to_numpy()
            y_b = np.ones_like(y_a)

            ax.bar(
                x - width,
                y_a,
                width,
                label=label_a if (row == 0 and col_idx == 0) else "",
                hatch="///",
            )

            ax.bar(
                x,
                y_b,
                width,
                label=label_b if (row == 0 and col_idx == 0) else "",
                hatch="xxx",
            )

            ax.axhline(1, linestyle="--", linewidth=0.8)
            ax.grid(True, linestyle=":", axis="y")

            if row == 0:
                ax.set_title(col.capitalize())

            if col_idx == 0:
                ax.set_ylabel(f"{payload_label}")

            all_vals = np.concatenate([y_a, y_b])
            all_vals = all_vals[np.isfinite(all_vals)]
            if len(all_vals) > 0:
                ratio = all_vals.max() / max(all_vals.min(), 1e-9)
                if ratio > 20:
                    ax.set_yscale("log")

            if row == payloads_nr - 1:
                ax.set_xticks(x)
                ax.set_xticklabels(clients, rotation=45)
            else:
                ax.set_xticks([])

    handles, labels = axs[0, 0].get_legend_handles_labels()
    fig.legend(
        handles,
        labels,
        loc="upper center",
        ncol=2,
        frameon=False,
        bbox_to_anchor=(0.5, 0.98),
    )

    fig.suptitle(
        f"Client Execution Time (Relative to {label_b})",
        fontsize=15,
    )

    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.savefig(
        f"{outdir}/combined_client_metrics_time_per_clients2_bar.pdf", format="pdf"
    )
    plt.close()


# def create_bar_plot2(
#     csv_paths1,
#     csv_paths2,
#     labels1=("AQUA", "UDS", "ZeroMQ"),
#     labels2=("AQUA", "UDS", "ZeroMQ"),
# ):
#     csv_a1, csv_b1, csv_c1 = csv_paths1
#     csv_a2, csv_b2, csv_c2 = csv_paths2

#     label_a1, label_b1, label_c1 = labels1
#     label_a2, label_b2, label_c2 = labels2

#     df_a1 = load_and_prepare(csv_a1, TIME_COLUMNS)
#     df_b1 = load_and_prepare(csv_b1, TIME_COLUMNS)
#     df_c1 = load_and_prepare(csv_c1, TIME_COLUMNS)

#     df_a2 = load_and_prepare(csv_a2, TIME_COLUMNS)
#     df_b2 = load_and_prepare(csv_b2, TIME_COLUMNS)
#     df_c2 = load_and_prepare(csv_c2, TIME_COLUMNS)


# def create_bar_plot3(
#     csv_paths1,
#     csv_paths2,
#     csv_paths3,
#     labels1=("AQUA", "UDS", "ZeroMQ"),
#     labels2=("AQUA", "UDS", "ZeroMQ"),
#     labels3=("AQUA", "UDS", "ZeroMQ"),
# ):
#     csv_a1, csv_b1, csv_c1 = csv_paths1
#     csv_a2, csv_b2, csv_c2 = csv_paths2
#     csv_a3, csv_b3, csv_c3 = csv_paths3

#     label_a1, label_b1, label_c1 = labels1
#     label_a2, label_b2, label_c2 = labels2
#     label_a3, label_b3, label_c3 = labels3

#     df_a1 = load_and_prepare(csv_a1)
#     df_b1 = load_and_prepare(csv_b1)
#     df_c1 = load_and_prepare(csv_c1)

#     df_a2 = load_and_prepare(csv_a2)
#     df_b2 = load_and_prepare(csv_b2)
#     df_c2 = load_and_prepare(csv_c2)

#     df_a3 = load_and_prepare(csv_a3)
#     df_b3 = load_and_prepare(csv_b3)
#     df_c3 = load_and_prepare(csv_c3)


def create_bar_plot3(
    payloads_csv_paths: dict[tuple[str]],
    labels: tuple[str] = ("AQUA", "UDS", "ZeroMQ"),
    outdir: str = ".",
):
    label_a, label_b, label_c = labels

    payloads_nr = len(payloads_csv_paths)
    metrics_nr = len(TIME_COLUMNS)

    fig, axs = plt.subplots(payloads_nr, metrics_nr, figsize=(16, 12), sharex="col")

    width = 0.2

    for row, (payload_label, csv_paths) in enumerate(payloads_csv_paths.items()):
        csv_a, csv_b, csv_c = csv_paths

        df_a = load_and_prepare(csv_a, TIME_COLUMNS)
        df_b = load_and_prepare(csv_b, TIME_COLUMNS)
        df_c = load_and_prepare(csv_c, TIME_COLUMNS)

        clients = df_a["clients_nr"].to_numpy()
        x = np.arange(len(clients))

        for col_idx, col in enumerate(TIME_COLUMNS):
            ax = axs[row, col_idx]

            y_a = (df_a[col] / df_b[col]).to_numpy()
            y_b = np.ones_like(y_a)
            y_c = (df_c[col] / df_b[col]).to_numpy()

            ax.bar(
                x - width,
                y_a,
                width,
                label=label_a if (row == 0 and col_idx == 0) else "",
                hatch="///",
            )

            ax.bar(
                x,
                y_b,
                width,
                label=label_b if (row == 0 and col_idx == 0) else "",
                hatch="xxx",
            )

            ax.bar(
                x + width,
                y_c,
                width,
                label=label_c if (row == 0 and col_idx == 0) else "",
                hatch="...",
            )

            ax.axhline(1, linestyle="--", linewidth=0.8)
            ax.grid(True, linestyle=":", axis="y")

            if row == 0:
                ax.set_title(col.capitalize())

            if col_idx == 0:
                ax.set_ylabel(f"{payload_label}")

            all_vals = np.concatenate([y_a, y_b, y_c])
            all_vals = all_vals[np.isfinite(all_vals)]
            if len(all_vals) > 0:
                ratio = all_vals.max() / max(all_vals.min(), 1e-9)
                if ratio > 20:
                    ax.set_yscale("log")

            if row == payloads_nr - 1:
                ax.set_xticks(x)
                ax.set_xticklabels(clients, rotation=45)
            else:
                ax.set_xticks([])

    handles, labels = axs[0, 0].get_legend_handles_labels()
    fig.legend(
        handles,
        labels,
        loc="upper center",
        ncol=3,
        frameon=False,
        bbox_to_anchor=(0.5, 0.98),
    )

    fig.suptitle(
        f"Client Execution Time (Relative to {label_b})",
        fontsize=15,
    )

    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.savefig(
        f"{outdir}/combined_client_metrics_time_per_clients2_bar.pdf", format="pdf"
    )
    plt.close()


# def create_bar_plot3(
#     csv_paths1,
#     csv_paths2,
#     csv_paths3,
#     labels1=("AQUA", "UDS", "ZeroMQ"),
#     labels2=("AQUA", "UDS", "ZeroMQ"),
#     labels3=("AQUA", "UDS", "ZeroMQ"),
# ):
#     csv_a1, csv_b1, csv_c1 = csv_paths1
#     csv_a2, csv_b2, csv_c2 = csv_paths2
#     csv_a3, csv_b3, csv_c3 = csv_paths3

#     label_a1, label_b1, label_c1 = labels1
#     label_a2, label_b2, label_c2 = labels2
#     label_a3, label_b3, label_c3 = labels3

#     df_a1 = load_and_prepare(csv_a1)
#     df_b1 = load_and_prepare(csv_b1)
#     df_c1 = load_and_prepare(csv_c1)

#     df_a2 = load_and_prepare(csv_a2)
#     df_b2 = load_and_prepare(csv_b2)
#     df_c2 = load_and_prepare(csv_c2)

#     df_a3 = load_and_prepare(csv_a3)
#     df_b3 = load_and_prepare(csv_b3)
#     df_c3 = load_and_prepare(csv_c3)


if __name__ == "__main__":
    create_bar_plot3(
        {
            "64KB": (
                "benchmark_results/clients/1772287597/client_benchmark.csv",
                "uds/benchmark_results/clients/1772287759/client_benchmark.csv",
                "zeromq/benchmark_results/clients/1772287928/client_benchmark.csv",
            ),
            "128KB": (
                "benchmark_results/clients/1772286859/client_benchmark.csv",
                "uds/benchmark_results/clients/1772287026/client_benchmark.csv",
                "zeromq/benchmark_results/clients/1772287197/client_benchmark.csv",
            ),
            "256KB": (
                "benchmark_results/clients/1772288952/client_benchmark.csv",
                "uds/benchmark_results/clients/1772289126/client_benchmark.csv",
                "zeromq/benchmark_results/clients/1772289301/client_benchmark.csv",
            ),
            "512KB": (
                "benchmark_results/clients/1772200354/client_benchmark.csv",
                "uds/benchmark_results/clients/1772200525/client_benchmark.csv",
                "zeromq/benchmark_results/clients/1772200735/client_benchmark.csv",
            ),
        }
    )

    # create_bar_plot2(
    #     {
    #         "64KB": (
    #             "benchmark_results/clients/1772287597/client_benchmark.csv",
    #             "uds/benchmark_results/clients/1772287759/client_benchmark.csv",
    #         ),
    #         "128KB": (
    #             "benchmark_results/clients/1772286859/client_benchmark.csv",
    #             "uds/benchmark_results/clients/1772287026/client_benchmark.csv",
    #         ),
    #         "256KB": (
    #             "benchmark_results/clients/1772288952/client_benchmark.csv",
    #             "uds/benchmark_results/clients/1772289126/client_benchmark.csv",
    #         ),
    #         "512KB": (
    #             "benchmark_results/clients/1772200354/client_benchmark.csv",
    #             "uds/benchmark_results/clients/1772200525/client_benchmark.csv",
    #         ),
    #     }
    # )
