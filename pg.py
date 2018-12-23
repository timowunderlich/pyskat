import pyskat
import numpy as np
import tensorflow as tf
from tensorflow._api.v1.keras import layers

class PolicyPlayer(pyskat.Player):
    # total input size = hole cards + friendly trick + hostile trick + friendly won + hostile won + is declarer
    input_size = 32 + 32 + 32 + 32 + 32 + 1 
    default_hparams = {"hidden_size": 100} 
    def __init__(self, hparams=default_hparams):
        super(PolicyPlayer, self).__init__()
        self.hparams = hparams
        # Create policy model
        input_layer = layers.Input(shape=(PolicyPlayer.input_size, ))
        hidden_layer = layers.Dense(hparams["hidden_size"], activation="relu")(input_layer)
        softmax_layer = layers.Dense(32, activation="softmax")(hidden_layer)
        self.model = tf.keras.Model(inputs=input_layer, outputs=softmax_layer)
        # Create training model that wraps above model
        reward_input = layers.Input(shape=(1,))
        self.training_model = tf.keras.Model(inputs=[input_layer, reward_input], outputs=softmax_layer)
        self.training_model.compile(optimizer="rmsprop", loss=self.create_loss_function(reward_input))
    
    # Creates loss function that takes reward into account 
    def create_loss_function(self, reward_input):
        # y_true are one-hot action vectors, y_pred are network outputs
        def lossfct(y_true, y_pred):
            # This masks out all not taken actions
            action_probs = tf.reduce_sum(y_true*y_pred, axis=1)
            loss = -tf.reduce_sum(reward_input * tf.log(action_probs))
            return loss
        return lossfct


    # Converts PlayerState into representation to be used as model input
    def convert_state_for_model(self, state):
        assert(isinstance(state, pyskat.PlayerState))
        # Sequentially create list with multi-hot card representation
        input_repr = list()
        input_repr.extend(pyskat.get_multi_hot(state.hole_cards)*np.ones(32))
        input_repr.extend(pyskat.get_multi_hot(state.trick_friendly)*np.ones(32))
        input_repr.extend(pyskat.get_multi_hot(state.trick_hostile)*np.ones(32))
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
        return np.array(before_states), np.array(actions), np.array(rewards)

    def train_on_transitions(self):
        states, actions, rewards = self.convert_transitions_for_model()
        if len(states) == 0:
            return
        self.training_model.train_on_batch([states, rewards], actions)
        print("Got average reward: {}".format(np.mean(rewards)))
        self.clear_transitions()

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
    def __init__(self):
        # The three skateers
        self.one = PolicyPlayer()
        self.two = PolicyPlayer()
        self.three = PolicyPlayer()
        self.players = [self.one, self.two, self.three]
        self.game = pyskat.Game(self.one, self.two, self.three, retry_on_illegal_action=False)

    def train(self, eps=1000, games_per_ep=100):
        for _ in range(eps):
            for _ in range(games_per_ep):
                trainer.game.run_new_game()
            for player in trainer.players:
                player.train_on_transitions()

if __name__ == "__main__":
    trainer = PlayerTrainer()
    trainer.train()
