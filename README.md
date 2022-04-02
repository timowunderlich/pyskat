![](logo.svg)

## Description
PySkat implements a subset of the [Skat card game](https://en.wikipedia.org/wiki/Skat_(card_game)): a suit game where ♣ is the trump.
It uses C++ code that is wrapped to Python using [pybind11](https://github.com/pybind/pybind11).

It also provides a `PlayerTrainer` class that implements a neural network using [Tensorflow](https://github.com/tensorflow/tensorflow) which learns to play using the [policy gradient method](http://www.scholarpedia.org/article/Policy_gradient_methods).


## Installation
To build from source, you need: Pip >=10, [boost](https://www.boost.org/), [googletest](https://github.com/google/googletest), a C++ compiler.
Clone the git repository, navigate to the root folder and run
```
pip install .
```

## Quick Start
The root directory contains two scripts to play against a neural network model and to train a model.
### Play Against Neural Network
Play against the neural network using the pre-trained model contained in `skat_model.h5`:
```
python play_against_model.py --model skat_model.h5
```
The pre-trained model seems to be able to make legal moves consistently. Its actual skill is another matter.

### Train Neural Network Using Policy Gradient
Train the model, starting from the pre-trained state:
```
python train_policy_gradient.py --start-model skat_model.h5
```

## Tests
Run, in the root folder, 
```
python setup.py test
```
