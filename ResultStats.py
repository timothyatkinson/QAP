import csv
import statistics as stts
import scipy.stats as spy
from itertools import islice
from VD_A import VD_A


class ResultStats:
    def __init__(self, algorithm_name, target_gate_name, target_fitness, result_file):
        self.algorithm = algorithm_name
        self.gate = target_gate_name
        with open(result_file) as h:
            self.fitness_list = [float(row[1]) for row in islice(csv.reader(h), 1, None)]
        self.success_rate = len([x for x in self.fitness_list if x >= target_fitness]) / len(self.fitness_list)
        self.median = stts.median(self.fitness_list)
        self.iqr = spy.iqr(self.fitness_list)

    def mann_whitney_u(self, other):
        return spy.mannwhitneyu(self.fitness_list, other.fitness_list).pvalue

    def vargha_delaney_a(self, other):
        return VD_A(self.fitness_list, other.fitness_list)[0]

    def __str__(self):
        return (
               f"{{ algorithm: {self.algorithm}\n"
               f", gate: {self.gate}\n"
               f", fitness_list: {self.fitness_list}\n"
               f", success_rate: {self.success_rate}\n"
               f", median: {self.median}\n"
               f", iqr: {self.iqr} }}"
        )


stats_qap = ResultStats("qap", "fredkin", 0.98, "output/fredkin/qap.csv")
stats_rqap = ResultStats("r-qap", "fredkin", 0.98, "output/fredkin/r-qap.csv")
print(stats_qap)
print(stats_rqap)
print("Mann Whitney U p: " + str(stats_qap.mann_whitney_u(stats_rqap)))
print("Vargha Delaney A: " + str(stats_qap.vargha_delaney_a(stats_rqap)))
