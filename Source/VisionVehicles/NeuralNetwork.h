// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "NeuralNetwork.generated.h"

/** This class implements a neural network used by the vehicles AI controller.
 *		The NN architecture is a Multi-Layer Perceptron (MLP).
 */
UCLASS(Blueprintable)
class VISIONVEHICLES_API UNeuralNetwork : public UObject
{
	GENERATED_BODY()

private:
	// Basic constructor. Private to enforce factory method.
	UNeuralNetwork();

	// The number of inputs of the neural network
	int nInputs;

	// The number of outputs of the neural network
	int nOutputs;

	// The dimension for the hidden layers in the neural network
	TArray<int> nHiddenLayers;

	// The weights for each layer of neural network, and for each neuron
	TArray<TArray<TArray<float>>> weights;

	// The learning rate used for training
	float learningRate;

	// The current epoche of training
	int epoche;

	// The initial value for the learning rate
	float initialLearningRate;

	// The factor that controls the decay of the learning rate
	float learningRateDecay;

public:
	// Factory method for the class
	UFUNCTION(BlueprintCallable, Category = "AI|Neural Network")
	static UNeuralNetwork* GetInstance();

	// Initializes the neural network with the specified dimensions
	UFUNCTION(BlueprintCallable, Category = "AI|Neural Network")
	void Init(int inputs, int outputs, TArray<int> hiddenLayers, float _initialLearningRate = 0.1f, float _learningRateDecay = 0.001f);

	// Runs the neural network for the given inputs and returns its ouput
	UFUNCTION(BlueprintCallable, Category = "AI|Neural Network")
	TArray<float> Run(TArray<float> inputs);

	/* Trains the neural network for the given inputs and expected output.
	 *	Returns the error that was made in this iteration. */
	UFUNCTION(BlueprintCallable, Category = "AI|Neural Network")
	float Train(TArray<float> inputs, TArray<float> expectedOutputs);

     TArray<int> GetStructure();

     float GetWeight(int _layerId, int _fromInd, int _toInd);
private:
	// Runs the neural network for the given inputs and returns its ouput
	//	Also returns the weighted sums and activations of each unit in each layer
	TArray<float> Run(TArray<float> inputs, TArray<TArray<float>>* weightedSums, TArray<TArray<float>>* activations);

	// Calculates the dot product of two vectors
	// PRE: a.Num() == b.Num()
	float Dot(const TArray<float>& a, const TArray<float>& b) const;

	// Computes the error between the two given errors
	float ComputeError(const TArray<float>& a, const TArray<float>& b) const;

	// Computes the difference of the two vectors: a - b
	TArray<float> Difference(const TArray<float>& a, const TArray<float>& b) const;

	// Computes the element-wise multiplication of two vectors
	TArray<float> Multiply(const TArray<float>& a, const TArray<float>& b) const;

	// Transposes the matrix
	// Pre: 'a' represents a proper 2-D matrix
	TArray<TArray<float>> Transpose(const TArray<TArray<float>>& a) const;

	// Reverses the vector
	template<typename T> TArray<T> Reverse(const TArray<T>& a) const;

	// The sigmoid function used as activation function for the neural network
	FORCEINLINE float Sigmoid(float value) const { return 1.0f / (1.0f + FMath::Exp(-value)); }

	// The derivative of the sigmoid function
	FORCEINLINE float SigmoidPrime(float value) const { float s = Sigmoid(value); return s * (1 - s); }

     
};
