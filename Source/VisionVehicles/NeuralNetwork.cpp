// Fill out your copyright notice in the Description page of Project Settings.

#include "VisionVehicles.h"
#include "NeuralNetwork.h"


UNeuralNetwork::UNeuralNetwork()
{

}

UNeuralNetwork* UNeuralNetwork::GetInstance()
{
	return NewObject<UNeuralNetwork>();
}

void UNeuralNetwork::Init(int inputs, int outputs, TArray<int> hiddenLayers)
{
	// Set dimensions
	nInputs = inputs;
	nOutputs = outputs;
	nHiddenLayers = hiddenLayers;

	TArray<int> dimensions;
	dimensions.Add(nInputs);
	dimensions.Append(nHiddenLayers);
	dimensions.Add(nOutputs);

	// Initialize weights
	weights.Empty();
	for (int l = 1; l < dimensions.Num(); l++)
	{
		TArray<TArray<float>> layer; // The weights for the current layer

		for (int j = 0; j < dimensions[l]; j++)
		{
			TArray<float> w; // The weights for the current unit
			for (int i = 0; i < dimensions[l - 1] + 1; i++) // +1: include the weight for the bias
			{
				w.Add(FMath::RandRange(-1.0f, 1.0f));
			}
			layer.Add(w);
		}

		weights.Add(layer);
	}
}

TArray<float> UNeuralNetwork::Run(TArray<float> inputs, TArray<TArray<float>>* weightedSums, TArray<TArray<float>>* activations)
{
	weightedSums->Empty();
	activations->Empty();

	// Initialize the first layer with the input values
	activations->Add(inputs);
	
	// Feed forward the activation values
	for (int l = 0; l < weights.Num(); l++)
	{
		TArray<float> ws; // The weighted sum for each unit in this layer
		TArray<float> a; // The activation for each unit in this layer

		TArray<float> prevActivation = (*activations)[l]; // The activations of the previous layer
		prevActivation.Add(1.0f); // Add the extra value for the bias
		for (int j = 0; j < weights[l].Num(); j++)
		{
			float z = Dot(prevActivation, weights[l][j]);
			ws.Add(z);
			a.Add(Sigmoid(z));
		}

		weightedSums->Add(ws);
		activations->Add(a);
	}

	// Return the output values
	return (*activations).Top();
}

TArray<float> UNeuralNetwork::Run(TArray<float> inputs)
{
	TArray<TArray<float>> weightedSums, activations;
	return Run(inputs, &weightedSums, &activations);
}

float UNeuralNetwork::Train(TArray<float> inputs, TArray<float> expectedOutputs)
{
	// Run the NN and get the resulted output
	TArray<TArray<float>> weightedSums, activations;
	TArray<float> outputs = Run(inputs, &weightedSums, &activations);

	// The deltas for each unit in each layer (how a change in its value affects a change in the error)
	TArray<TArray<float>> deltas;

	// Calculate the error made
	float error = ComputeError(expectedOutputs, outputs);

	return error;
}

float UNeuralNetwork::Dot(const TArray<float>& a, const TArray<float>& b) const
{
	if (a.Num() != b.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to dot product two vectors with non-equal dimensions: %d - %d."), a.Num(), b.Num());
		return 0.0f;
	}

	float value = 0.0f;
	for (int i = 0; i < a.Num(); i++)
	{
		value += a[i] * b[i];
	}
	return value;
}

float UNeuralNetwork::ComputeError(const TArray<float>& a, const TArray<float>& b) const
{
	if (a.Num() != b.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to compare two vectors with non-equal dimensions: %d - %d."), a.Num(), b.Num());
		return 0.0f;
	}

	float value = 0.0f;
	for (int i = 0; i < a.Num(); i++)
	{
		value += (a[i] - b[i]) * (a[i] - b[i]);
	}
	return value * 0.5f;
}