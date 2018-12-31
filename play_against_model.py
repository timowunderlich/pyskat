import pyskat
import numpy as np
import tensorflow as tf
from tensorflow._api.v1.keras import models
from pg import PolicyPlayer
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--model", type=str, required=True, help="HDF5 model filename.")
    parser.add_argument("--rounds", type=int, default=10, help="Number of rounds to play.")
    args = parser.parse_args()
    model = models.load_model(args.model)
    p1 = PolicyPlayer(model, None) # no training model required
    p2 = PolicyPlayer(model, None)
    human = pyskat.HumanPlayer()
    game = pyskat.Game(p1, p2, human, max_rounds=args.rounds, retry_on_illegal_action=True)
    game.run_new_game()
