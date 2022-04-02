import argparse
from pyskat.training import PlayerTrainer


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--start-model", type=str, help="Starting model filename (HDF5).")
    args = parser.parse_args()
    trainer = PlayerTrainer(start_model=args.start_model, save_to="skat_model.h5")
    trainer.train()
