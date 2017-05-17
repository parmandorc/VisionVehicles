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
	nInputs = inputs;
	nOutputs = outputs;
	nHiddenLayers = hiddenLayers;
}

TArray<float> UNeuralNetwork::Run(TArray<float> inputs)
{
	TArray<float> output;
	output.Init(0.0f, inputs.Num());
	return output;
}