#!/usr/bin/env python3
from array import array

from keras.callbacks import ModelCheckpoint, BackupAndRestore, Callback, ReduceLROnPlateau, CSVLogger
from keras.models import Model
from keras import layers
import keras
from tensorflow.keras import initializers
from tensorflow.data import Dataset
import tensorflow as tf
import numpy as np
import struct
import math
import os
import json

FEN_FILENAME = 'fen.data'

BATCH_SIZE = 16384

EPOCH_SIZE = 10000000
EPOCHS = 300

INPUT_SIZE = 768
ACCUMULATOR_SIZE = 512
SCALING = 50000.0

class SaveWeights(Callback):
    def __init__(self):
        super().__init__()

    def on_epoch_end(self, epoch, logs=None):

        first_layer_weights = self.model.layers[2].get_weights()[0]
        first_layer_biases  = self.model.layers[2].get_weights()[1]
        second_layer_weights = self.model.layers[4].get_weights()[0]
        second_layer_biases  = self.model.layers[4].get_weights()[1]

        output_file = open('file' + str(epoch+1) +'.weight' , 'wb')
        first_layer_biases.tofile(output_file)
        first_layer_weights.tofile(output_file)
        second_layer_biases.tofile(output_file)
        second_layer_weights.tofile(output_file)
        output_file.close()


def get_train_data_size(filename):
    with open(filename, 'rb') as f:
        #print("RESTART")
        count = 0
        while True:
            fs =f.read(1)
            if not fs:
                break
            count +=1
            #print(count)
            featuresSize = int.from_bytes(fs, byteorder='little')
            ff = f.read(2*featuresSize)
            ff = f.read(4)
        return count

firstRun = True
def read_train_data(filename, skip):
    global firstRun

    with open(filename, 'rb') as f:
        print("RESTART")
        count = 0
        if firstRun:
            print("skipping {} positions".format(skip))
            firstRun = False
            while skip>0:
                fs =f.read(1)
                if not fs:
                    break
                count +=1
                #print(count)
                featuresSize = int.from_bytes(fs, byteorder='little')
                ff = f.read(2*featuresSize)
                ff = f.read(4)
                skip -= 1

        while True:
            fs = f.read(1)
            if not fs:
                break
            count +=1
            #print(count)
            featuresSize = int.from_bytes(fs, byteorder='little')

            feature = np.zeros(INPUT_SIZE)
            feature2 = np.zeros(INPUT_SIZE)

            #debugf = []
            #debugf2 = []

            ff = f.read(2*featuresSize)
            for i in range(featuresSize):
                bites = ff[2*i:2*i+2]
                #bites = f.read(2)
                idx = int.from_bytes(bites, byteorder='little')
                feature[idx] = 1
                #debugf.append(idx)

                if idx <384:
                    idx +=384
                else:
                    idx -=384
                idx = idx ^ 0b111000
                feature2[idx] = 1
                #debugf2.append(idx)

            #    #print(idx)
            #print(debugf)
            #debugf2.sort()
            #print(debugf2)

            label = struct.unpack('<f', f.read(4))[0] / SCALING
            label = 1.0/(1 + math.exp(-1.0 * label)) ;
            #float.from_bytes(f.read(4), byteorder='little')
            #print(label)
            yield (feature, feature2), label
            #exit(0)

def get_dataset(skip):
    filename = FEN_FILENAME
    generator = lambda: read_train_data(filename, skip)
    return Dataset.from_generator(
        generator, ((tf.int32,tf.int32), tf.float32), (((INPUT_SIZE,),(INPUT_SIZE,)), ()))


input1 = keras.Input(shape=(INPUT_SIZE,),dtype='int32')
input2 = keras.Input(shape=(INPUT_SIZE,),dtype='int32')
dense = layers.Dense(ACCUMULATOR_SIZE, activation='relu')
x = dense(input1)
y = dense(input2)
merged = layers.Concatenate(axis=1)([x, y])
output = layers.Dense(1, activation='sigmoid')(merged)
model = Model(inputs=[input1, input2], outputs=output)

model.compile(loss='mean_squared_error',
              optimizer='adamw')

model.summary()

size = get_train_data_size(FEN_FILENAME)
print("training data positions {}".format(size))

print("*** Training... ***")

spe = int(EPOCH_SIZE/BATCH_SIZE)

skip = 0
epoch = 0

path = os.path.join(os.getcwd(), 'backup', 'training_metadata.json')
if os.path.exists(path):
    with open(path, 'r') as file:
        epoch = json.load(file)['epoch']

print("epoch {}".format(epoch))

skip = epoch * spe * BATCH_SIZE
skip = skip % size

data = get_dataset(skip).batch(BATCH_SIZE).repeat()

backup = BackupAndRestore(backup_dir="./backup")
reduce_lr = ReduceLROnPlateau(monitor='loss', factor=0.5, patience=10, verbose=1)
csv_logger = CSVLogger('training.log', separator=",", append=True)

model.fit(data, epochs=EPOCHS, verbose=1,steps_per_epoch= spe, callbacks=[backup, SaveWeights(), reduce_lr, csv_logger])


print("*** Training done! ***")
