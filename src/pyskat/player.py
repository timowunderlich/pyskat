import pyskat_cpp
import numpy as np

class PolicyPlayer(pyskat_cpp.Player):
    # total input size = hole cards + trick card 1 + trick card 2 + friendly won + hostile won + is declarer
    input_size = 32 + 32 + 32 + 32 + 32 + 1
    def __init__(self, policy_model, training_model):
        super(PolicyPlayer, self).__init__()
        self.model = policy_model
        self.training_model = training_model
        self.counter = 0

    # Converts PlayerState into representation to be used as model input
    def convert_state_for_model(self, state):
        assert(isinstance(state, pyskat_cpp.PlayerState))
        trick_card_1 = np.array(state.trick[0].to_one_hot())*1. if len(state.trick) > 0 else np.zeros(32)
        trick_card_2 = np.array(state.trick[1].to_one_hot())*1. if len(state.trick) > 1 else np.zeros(32)
        # Sequentially create list with multi-hot card representation
        input_repr = list()
        input_repr.extend(pyskat_cpp.get_multi_hot(state.hole_cards)*np.ones(32))
        input_repr.extend(trick_card_1)
        input_repr.extend(trick_card_2)
        input_repr.extend(pyskat_cpp.get_multi_hot(state.won_friendly)*np.ones(32))
        input_repr.extend(pyskat_cpp.get_multi_hot(state.won_hostile)*np.ones(32))
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
        card = pyskat_cpp.Card(card_id)
        self.counter += 1
        return card