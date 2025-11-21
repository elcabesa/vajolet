#!/usr/bin/env python3
from array import array

from keras.callbacks import ModelCheckpoint, BackupAndRestore, Callback, ReduceLROnPlateau
from keras.models import Sequential
from tensorflow.keras.layers import Dense, Activation, Input
from tensorflow.keras import initializers
from tensorflow.data import Dataset
import tensorflow as tf
import numpy as np
import struct
import math

class SaveWeights(Callback):
    def __init__(self):
        super().__init__()

    def on_epoch_end(self, epoch, logs=None):

        first_layer_weights = self.model.layers[0].get_weights()[0]
        first_layer_biases  = self.model.layers[0].get_weights()[1]
        second_layer_weights = self.model.layers[1].get_weights()[0]
        second_layer_biases  = self.model.layers[1].get_weights()[1]
        output_file = open('file' + str(epoch+1) +'.weight' , 'wb')
        first_layer_biases.tofile(output_file)
        first_layer_weights.tofile(output_file)
        second_layer_biases.tofile(output_file)
        second_layer_weights.tofile(output_file)
        output_file.close()


def read_train_data(filename):
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

            feature = np.zeros(768)
            ff = f.read(2*featuresSize)
            for i in range(featuresSize):
                bites = ff[2*i:2*i+2]
                #bites = f.read(2)
                idx = int.from_bytes(bites, byteorder='little')
                feature[idx] = 1
            #    #print(idx)
            label = struct.unpack('<f', f.read(4))[0] /50000.0
            label = 1.0/(1 + math.exp(-1.0 * label)) ;
            #float.from_bytes(f.read(4), byteorder='little')
            #print(label)
            yield feature, label
            #exit(0)

def get_dataset():
    filename = 'fen.data'
    generator = lambda: read_train_data(filename)
    return Dataset.from_generator(
        generator, (tf.int32, tf.float32), ((768,), ()))


model = Sequential()
model.add(Input(shape=(768,),dtype='int32'))
model.add(Dense(512, activation='relu'))
model.add(Dense(1, activation='sigmoid'))

model.compile(loss='mean_squared_error',
              optimizer='adamw',
              metrics=['accuracy'])

model.summary()

print("*** Training... ***")

data = get_dataset().batch(16348).repeat()



backup = BackupAndRestore(backup_dir="./backup")
reduce_lr = ReduceLROnPlateau(monitor='loss', factor=0.5, patience=10, verbose=1)

spe = int(10000000/16348)

model.fit(data, epochs=200, verbose=1,steps_per_epoch= spe, callbacks=[backup, SaveWeights(), reduce_lr])


print("*** Training done! ***")




