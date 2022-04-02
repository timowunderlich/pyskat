import pyskat
import numpy as np
import tensorflow as tf
from tensorflow.keras import layers, models

from .player import PolicyPlayer


class PlayerTrainer(object):
    # total input size = hole cards + trick card 1 + trick card 2 + friendly won + hostile won + is declarer
    input_size = 32 + 32 + 32 + 32 + 32 + 1
    default_hparams = {"hidden_size_1": input_size, "hidden_size_2": input_size, "hidden_size_3": 100, "hidden_size_4": 100}
    def __init__(self, hparams=default_hparams, save_to=None, start_model=None):
        self.hparams = hparams
        self.save_to = save_to
        self.states = list()
        self.rewards = list()
        self.actions = list()
        if start_model is None:
            # Create policy model
            input_layer = layers.Input(shape=(PolicyPlayer.input_size, ))
            hidden_layer = layers.Dense(hparams["hidden_size_1"], activation="relu")(input_layer)
            hidden_layer = layers.Dense(hparams["hidden_size_2"], activation="relu")(hidden_layer)
            hidden_layer = layers.Dense(hparams["hidden_size_3"], activation="relu")(hidden_layer)
            hidden_layer = layers.Dense(hparams["hidden_size_4"], activation="relu")(hidden_layer)
            softmax_layer = layers.Dense(32, activation="softmax")(hidden_layer)
            self.model = tf.keras.Model(inputs=input_layer, outputs=softmax_layer)
            # Create training model that wraps above model
            reward_input = layers.Input(shape=(1,))
            self.training_model = tf.keras.Model(inputs=[input_layer, reward_input], outputs=softmax_layer)
            self.training_model.compile(optimizer="rmsprop", loss=self.create_loss_function(reward_input))
        else:
            self.model = models.load_model(start_model)
            input_layer = layers.Input(shape=(PolicyPlayer.input_size, ))
            reward_input = layers.Input(shape=(1,))
            self.training_model = tf.keras.Model(inputs=[input_layer, reward_input], outputs=self.model(input_layer))
            self.training_model.compile(optimizer="rmsprop", loss=self.create_loss_function(reward_input))

        # The three skateers
        self.one = PolicyPlayer(self.model, self.training_model)
        self.two = PolicyPlayer(self.model, self.training_model)
        self.three = PolicyPlayer(self.model, self.training_model)
        self.players = [self.one, self.two, self.three]
        self.game = pyskat.Game(self.one, self.two, self.three, retry_on_illegal_action=True)
        self.game.set_log_level_to_warning()

    # Creates loss function that takes reward into account
    def create_loss_function(self, reward_input):
        # y_true are one-hot action vectors, y_pred are network outputs
        def lossfct(y_true, y_pred):
            # This masks out all not taken actions
            action_probs = tf.keras.backend.sum(y_true*y_pred, axis=1)
            log_probs = tf.log(action_probs)
            rewards = tf.squeeze(reward_input)
            prod = tf.multiply(rewards, log_probs)
            loss = -tf.keras.backend.sum(prod)
            return loss
        return lossfct

    def discount_rewards(self, rewards):
        new_rewards = list()
        for r in rewards:
            new_rewards.append(rewards[-1])
        return new_rewards
        

    def save_transitions(self):
        for player in self.players:
            new_states, new_actions, new_rewards = player.get_transitions_for_model()
            self.states.extend(new_states)
            self.actions.extend(new_actions)
            self.rewards.extend(self.discount_rewards(new_rewards))

    def train_on_transitions(self):
        np_states, np_actions, np_rewards = np.array(self.states), np.array(self.actions), np.array(self.rewards)
        self.training_model.train_on_batch([np_states, np_rewards], np_actions)
        print("Got average reward: {}".format(np.mean(self.rewards)))

    def train(self, eps=10000, games_per_ep=1000):
        for ep in range(eps):
            for _ in range(games_per_ep):
                self.game.run_new_game()
                self.save_transitions()
            print("In episode {}".format(ep))
            self.train_on_transitions()
            if self.save_to is not None:
                self.model.save(self.save_to)