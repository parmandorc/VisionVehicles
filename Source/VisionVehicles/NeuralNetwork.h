// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/NoExportTypes.h"
#include "NeuralNetwork.generated.h"

/** This class implements a neural network used by the vehicles AI controller.
 *		The NN architecture is a Multi-Layer Perceptron (MLP).
 */
UCLASS()
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

public:
	// Factory method for the class
	static UNeuralNetwork* GetInstance();

	// Initializes the neural network with the specified dimensions
	void Init(int inputs, int outputs, TArray<int> hiddenLayers);

	// Runs the neural network for the given inputs and returns its ouput
	TArray<float> Run(TArray<float> inputs);

	/* Trains the neural network for the given inputs and expected output.
	 *	Returns the error that was made in this iteration. */
	float Train(TArray<float> inputs, TArray<float> expectedOutputs);

private:
	// Calculates the dot product of two vectors
	// PRE: a.Num() == b.Num()
	float Dot(const TArray<float>& a, const TArray<float>& b) const;

	// The sigmoid function used as activation function for the neural network
	FORCEINLINE float Sigmoid(float value) const { return 1.0f / (1.0f + FMath::Exp(-value)); }

	// Computes the error between the two given errors
	float ComputeError(const TArray<float>& a, const TArray<float>& b) const;
};
