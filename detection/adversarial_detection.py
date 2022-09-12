# Deep Learning Libraries
import numpy as np
np.set_printoptions(suppress=True)
from keras.models import load_model
from scipy.special import expit, softmax
import tensorflow as tf
tf.compat.v1.disable_eager_execution()
import keras.backend as K

class AdversarialDetection:
    def __init__(self, model, attack_type, classes,learning_rate = 2 / 255.0, batch = 1, decay = 0.98):
        self.classes = len(classes)
        self.epsilon = 1
        self.graph = tf.compat.v1.get_default_graph()
        self.use_filter = False

        self.noise = np.zeros((416, 416, 3))

        self.adv_patch_boxes = []
        self.fixed = True


        self.lr = learning_rate
        self.delta = 0
        self.decay = decay

        self.current_batch = 0
        self.batch = batch

        self.model = load_model(model)
        self.model.summary()
        self.attack_type = attack_type

        self.delta = None
        loss = 0

        for out in self.model.output:
            out = K.reshape(out, (-1, 5 + self.classes))

            # Targeted One Box
            if attack_type == "one_targeted":
                loss = K.max(K.sigmoid(K.reshape(out, (-1, 5 + self.classes))[:, 4]) * K.sigmoid(K.reshape(out, (-1, 5 + self.classes))[:, 5]))

            # Targeted Multi boxes
            if attack_type == "multi_targeted":
                loss = K.sigmoid(K.reshape(out, (-1, 5 + self.classes))[:, 4]) * K.sigmoid(K.reshape(out, (-1, 5 + self.classes))[:, 5])

            # Untargeted Multi boxes
            if attack_type == "multi_untargeted":
                for i in range(0, self.classes):
                    # PC Attack
                    loss = loss + tf.reduce_sum( K.sigmoid(out[:, 4]) * K.sigmoid(out[:, i+5]))

                    # Small centric
                    # loss = loss + tf.reduce_sum( K.sigmoid(out[:, 4]) * K.sigmoid(out[:, i+5]) / K.pow(K.sigmoid(out[:, 2]) * K.sigmoid(out[:, 3]), 2) )

                    # Large overlapped boxes
                    # loss = loss + tf.reduce_sum( K.sigmoid(out[:, 4]) * K.sigmoid(out[:, i+5])) / tf.reduce_sum(K.pow(K.sigmoid(out[:, 2]) * K.sigmoid(out[:, 3]), 2))

                # Small distributed boxes (PCB Attack)
                loss = loss / tf.reduce_sum(K.pow(K.sigmoid(out[:, 2]) * K.sigmoid(out[:, 3]), 2))

            grads = K.gradients(loss, self.model.input)

            if self.delta == None:
                self.delta =  K.sign(grads[0])
            else:
                self.delta = self.delta + K.sign(grads[0])

        # Store current patches
        self.patches = []

        # loss = K.sum(K.abs((self.model.input-K.mean(self.model.input))))

        # Reduce Random Noises
        # loss = - 0.01 * tf.reduce_sum(tf.image.total_variation(self.model.input))

        # Mirror
        # loss = - 0.01 * tf.reduce_sum(tf.image.total_variation(self.model.input)) - 0.01 * tf.reduce_sum(K.abs(self.model.input - tf.image.flip_left_right(self.model.input)))

        grads = K.gradients(loss, self.model.input)
        self.delta = self.delta + K.sign(grads[0])

        self.sess = tf.compat.v1.keras.backend.get_session()

    # Deep Fool: Project on the lp ball centered at 0 and of radius xi
    def proj_lp(self, v, xi=50, p=2):

        # SUPPORTS only p = 2 and p = Inf for now
        if p == 2:
            v = v * min(1, xi/np.linalg.norm(v.flatten('C')))
            # v = v / np.linalg.norm(v.flatten(1)) * xi
        elif p == np.inf:
            v = np.sign(v) * np.minimum(abs(v), xi)
        else:
            raise ValueError('Values of p different from 2 and Inf are currently not supported...')

        return v

    def attack(self, input_cv_image):
        with self.graph.as_default():
            # Draw each adversarial patch on the input image
            input_cv_image = input_cv_image + self.noise
            input_cv_image = np.clip(input_cv_image, 0.0, 1.0).astype(np.float32)

            if not self.fixed:
                outputs, grads = self.sess.run([self.model.output, self.delta], feed_dict={self.model.input:np.array([input_cv_image])})

                self.noise = self.noise + self.lr * grads[0, :, :, :]

                self.current_batch = self.current_batch + 1
                
                if self.current_batch == self.batch: 
                    self.lr = self.lr * self.decay
                    self.current_batch = 0

                self.noise = np.clip(self.noise, -1.0, 1.0)

                self.noise = self.proj_lp(self.noise, xi=8/255.0, p = np.inf)
            else:
                outputs = self.sess.run(self.model.output, feed_dict={self.model.input:np.array([input_cv_image])})

            return input_cv_image, outputs
