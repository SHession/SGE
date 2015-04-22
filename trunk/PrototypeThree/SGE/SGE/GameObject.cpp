#include "stdafx.h"
#include "SGE.h"

using namespace SGE::Framework;

GameObject::GameObject(){
	GameObject::position.x = GameObject::position.y = GameObject::position.z = GameObject::position.w = 0;

	GameObject::rotation.x = GameObject::rotation.y = GameObject::rotation.z = GameObject::rotation.w = 0;

	GameObject::scale.x = GameObject::scale.y = GameObject::scale.z = GameObject::scale.w = 1;

	GameObject::initialRotation= SGE::Vector4(0,0,1,0);
	
	GameObject::currentRotation = initialRotation;

	_mesh.index = -1;
	_texture.index = -1;
}

GameObject::GameObject(SGE::Vector4 position, SGE::Vector4 rotation , SGE::Vector4 scale , SGE::Graphics::Mesh mesh, SGE::Graphics::Texture texture){
	GameObject::position = position;
	GameObject::rotation = rotation;
	GameObject::scale = scale;

	GameObject::initialRotation.x = 0;
	GameObject::initialRotation.y = 0;
	GameObject::initialRotation.z = 1;
	GameObject::initialRotation.w = 0;
	
	GameObject::currentRotation.x = 0;
	GameObject::currentRotation.y = 0;
	GameObject::currentRotation.z = 0;
	GameObject::currentRotation.w = 0;

	_mesh = mesh;
	_texture = texture;
}

GameObject::~GameObject(){


}

void GameObject::Update(GameTime gameTime){


}

void GameObject::MoveTo(double x, double y, double z){
	position.x = x;
	position.y = y;
	position.z = z;
};
void GameObject::MoveBy(double x, double y, double z){
	position.x += x;
	position.y += y;
	position.z += z;
};
void GameObject::RotateTo(double x, double y, double z){
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
};
void GameObject::RotateBy(double x, double y, double z){
	rotation.x += x;
	rotation.y += y;
	rotation.z += z;
};
void GameObject::ScaleTo(double x, double y, double z){
	scale.x = x;
	scale.y = y;
	scale.z = z;
};
void GameObject::ScaleBy(double x, double y, double z){
	scale.x += x;
	scale.y += y;
	scale.z += z;
};

void GameObject::MoveForwardBy(double distance){
	position.x += currentRotation.x * distance; 
	position.y += currentRotation.y * distance;
	position.z += currentRotation.z * distance; 

}