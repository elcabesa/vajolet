#!/usr/bin/env python3
from array import array

from keras.callbacks import ModelCheckpoint, BackupAndRestore, Callback, ReduceLROnPlateau
from keras.models import Sequential
from tensorflow.keras.layers import Dense, Activation, Input
from tensorflow.keras import initializers
from tensorflow.data import Dataset
import tensorflow as tf
import numpy as np

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
    #while True:
        with open(filename, 'r') as f:
            count = 0
            for line in f.readlines():
                count +=1
                #print(count)
                record = line.rstrip().split(';')
                index_string = record[:-1]
                indexes = index_string[0].rstrip().split(' ')
                indexes = [int(n) for n in indexes]
                feature = [0] *768

                for n in indexes:
                    feature[n] = 1

                label= float(record[-1])
                yield feature, label

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
reduce_lr = ReduceLROnPlateau(monitor='loss', factor=0.9, patience=5, verbose=1)

spe = int(10000000/16348)

model.fit(data, epochs=100, verbose=1,steps_per_epoch= spe, callbacks=[backup, SaveWeights(), reduce_lr])


print("*** Training done! ***")




