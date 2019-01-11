import pyskat
import numpy as np
import tensorflow as tf
from tensorflow._api.v1.keras import layers, models
import argparse

class PolicyPlayer(pyskat.Player):
    # total input size = hole cards + trick card 1 + trick card 2 + friendly won + hostile won + is declarer
    input_size = 32 + 32 + 32 + 32 + 32 + 1
    def __init__(self, policy_model, training_model):
        super(PolicyPlayer, self).__init__()
        self.model = policy_model
        self.training_model = training_model

    # Converts PlayerState into representation to be used as model input
    def convert_state_for_model(self, state):
        assert(isinstance(state, pyskat.PlayerState))
        trick_card_1 = np.array(state.trick[0].to_one_hot())*1. if len(state.trick) > 0 else np.zeros(32)
        trick_card_2 = np.array(state.trick[1].to_one_hot())*1. if len(state.trick) > 1 else np.zeros(32)
        # Sequentially create list with multi-hot card representation
        input_repr = list()
        input_repr.extend(pyskat.get_multi_hot(state.hole_cards)*np.ones(32))
        input_repr.extend(trick_card_1)
        input_repr.extend(trick_card_2)
        input_repr.extend(pyskat.get_multi_hot(state.won_friendly)*np.ones(32))
        input_repr.extend(pyskat.get_multi_hot(state.won_hostile)*np.ones(32))
        input_repr.append(state.is_declarer * 1.)
        assert(PolicyPlayer.input_size == len(input_repr))
        return np.array(input_repr)

    # Converts transitions saved in player into representation for model
    def convert_transitions_for_model(self):
        transitions = self.get_transitions()
        before_states = list()
        actions = list()
        rewards = list()
        for t in transitions:
            before_states.append(self.convert_state_for_model(t.before))
            actions.append(t.action.to_one_hot()*np.ones(32))
            rewards.append(t.reward)
        return before_states, actions, rewards

    def get_transitions_for_model(self):
        states, actions, rewards = self.convert_transitions_for_model()
        self.clear_transitions()
        return states, actions, rewards

    # Is called by Game to get player's action, overrides pure virtual
    def query_policy(self):
        # Get probabilities from neural network
        state = self.convert_state_for_model(self.get_last_state())
        probs = self.model.predict(np.expand_dims(state, axis=0))[0]
        # Get one-hot representation of random card
        card_id = np.random.choice(32, p=probs)
        card = pyskat.Card(card_id)
        return card

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
        self.game = pyskat.Game(self.one, self.two, self.three, retry_on_illegal_action=False)

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

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--start-model", type=str, help="Starting model filename (HDF5).")
    args = parser.parse_args()
    trainer = PlayerTrainer(start_model=args.start_model, save_to="skat_model.h5")
    trainer.train()
