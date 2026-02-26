import sys
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def load_and_prepare(csv_path, time_columns):
    df = pd.read_csv(csv_path)

    # Convert microseconds -> ms
    df[time_columns] = df[time_columns] / 1000.0

    # Group by number of clients
    df_grouped = df.groupby("clients_nr")[time_columns].mean().reset_index()
    df_grouped = df_grouped.sort_values("clients_nr")

    return df_grouped

def create_bar_plot2(csv_paths):
    csv_a, csv_b, csv_c = csv_paths

    load_and_prepare(csv_a)
    load_and_prepare(csv_b)
    load_and_prepare(csv_c)

def create_bar_plot3():
    pass

if __name__ == "__main__":
    pass
