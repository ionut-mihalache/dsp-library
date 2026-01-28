import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import re
import sys


# --- 1. Parse the LaTeX Table ---
# def parse_latex_table(latex_str):
#     lines = latex_str.strip().split("\n")
#     data_rows = []
#     headers = []

#     for line in lines:
#         line = line.strip()
#         if (
#             not line
#             or line.startswith("\\")
#             or "toprule" in line
#             or "midrule" in line
#             or "bottomrule" in line
#         ):
#             continue

#         parts = [part.strip() for part in re.split(r"&|\\\\", line) if part.strip()]

#         if "clients" in line.lower():
#             headers = parts
#         elif parts and all(re.match(r"^-?\d+(\.\d+)?$", p) for p in parts[1:]):
#             data_rows.append(parts)

#     df = pd.DataFrame(data_rows, columns=headers)
#     df = df.apply(pd.to_numeric)  # convert all to numeric
#     return df


def parse_latex_table(latex_str):
    lines = latex_str.strip().splitlines()
    headers = []
    data_rows = []
    found_header = False

    for line in lines:
        line = line.strip()

        # Skip LaTeX/formatting lines
        if (
            not line
            or line.startswith("\\")
            or any(x in line for x in ["toprule", "midrule", "bottomrule"])
        ):
            continue

        # Split into columns
        parts = [p.strip() for p in re.split(r"&|\\\\", line) if p.strip()]

        # First actual data line is the header
        if not found_header:
            headers = parts
            found_header = True
            continue

        # Convert data cells
        row = []
        for p in parts:
            p = p.strip()
            if re.match(r"<\s*\d+(\.\d+)?", p):
                row.append(0.0)  # Replace < 1 or < 0.01 with 0.0
            else:
                try:
                    row.append(float(p))
                except ValueError:
                    row.append(np.nan)
        data_rows.append(row)

    df = pd.DataFrame(data_rows, columns=headers)
    return df


def create_exec_time_plot():
    with open(sys.argv[2]) as latex_table_file:
        latex_table = latex_table_file.read()

        # --- 3. Parse and plot ---
        df = parse_latex_table(latex_table)

        plt.style.use("seaborn-v0_8-whitegrid")
        styles = [
            {"linestyle": "-", "marker": "o"},
            {"linestyle": "--", "marker": "s"},
            {"linestyle": ":", "marker": "^"},
        ]

        plt.figure(figsize=(6.5, 4.5))

        for i, col in enumerate(["connect", "disconnect", "receive"]):
            plt.plot(
                df["clients"],
                df[col],
                label=col,
                **styles[i],
                linewidth=2,
                markersize=6,
            )

        plt.title("Execution Time Breakdown vs Number of Clients", fontsize=12)
        plt.xlabel("Number of Clients")
        plt.ylabel("Execution Time (%)")
        plt.xticks(df["clients"], rotation=45)
        plt.grid(True, linestyle="--", linewidth=0.5)
        plt.legend(frameon=False)
        plt.tight_layout()

        plt.savefig(sys.argv[1] + "/execution_time_from_latex.pdf", format="pdf")


def create_alloc_procentage_plot():
    with open(sys.argv[3]) as latex_table_file:
        latex_table = latex_table_file.read()

        # --- 3. Parse the table into a DataFrame ---
        df = parse_latex_table(latex_table)

        # --- 4. Plotting Setup (with color + line styles) ---
        plt.style.use("seaborn-v0_8-whitegrid")

        # Define styles (colors auto-assigned by matplotlib)
        line_styles = ["-", "--", ":"]
        markers = ["o", "s", "^"]
        colors = [
            "tab:blue",
            "tab:orange",
            "tab:green",
        ]  # you can customize these if you want

        # --- 5. Plot ---
        plt.figure(figsize=(6.5, 4.5))

        for i, col in enumerate(["connect", "receive", "send"]):
            plt.plot(
                df["clients"],
                df[col],
                label=col,
                linestyle=line_styles[i % len(line_styles)],
                marker=markers[i % len(markers)],
                linewidth=2,
                markersize=6,
                color=colors[i % len(colors)],
            )

        plt.title("Memory Allocation Breakdown vs Number of Clients", fontsize=12)
        plt.xlabel("Number of Clients")
        plt.ylabel("Allocation (%)")
        plt.xticks(df["clients"], rotation=45)
        plt.legend(frameon=False)
        plt.tight_layout()

        # Save and show
        plt.savefig(sys.argv[1] + "/alloc_from_latex.pdf", format="pdf")


def create_client_exec_time_plot():
    # Load CSV
    df = pd.read_csv(sys.argv[4])

    # Convert microseconds to milliseconds
    time_columns = ["connect", "call", "return", "disconnect"]
    df[time_columns] = df[time_columns] / 1000.0

    # Remove the outlier (clients_nr == 500)
    # df = df[df["clients_nr"] != 500]

    # Group by clients_nr and calculate the mean
    df_grouped = df.groupby("clients_nr")[time_columns].mean().reset_index()
    df_grouped = df_grouped.sort_values("clients_nr")

    # Define line styles and markers
    line_styles = ["-", "--", "-.", ":"]
    markers = ["o", "s", "^", "d"]

    # Set up subplots
    fig, axs = plt.subplots(len(time_columns), 1, figsize=(10, 10), sharex=True)

    for i, col in enumerate(time_columns):
        axs[i].plot(
            df_grouped["clients_nr"],
            df_grouped[col],
            linestyle=line_styles[i % len(line_styles)],
            marker=markers[i % len(markers)],
            color="black",
            label=col.capitalize(),
        )
        axs[i].set_ylabel("Time (ms)")
        # axs[i].set_title(f"{col.capitalize()} Time vs Number of Clients")
        axs[i].grid(True)

        # Move legend to the right of the plot
        axs[i].legend(frameon=False)

    axs[-1].set_xlabel("Number of Clients")
    plt.suptitle("Client Metrics vs Number of Clients (Averaged, ms)", fontsize=14)

    # Adjust layout to make space for legends on the right
    # plt.tight_layout(rect=[0, 0, 0.85, 0.95])
    plt.tight_layout()

    plt.savefig(sys.argv[1] + "/client_metrics_time_per_clients_bw.pdf", format="pdf")


# create_exec_time_plot()
# create_alloc_procentage_plot()
create_client_exec_time_plot()
