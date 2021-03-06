/*
*	 The MIT License (MIT)
*
*	 Copyright (c) 2015 Alisa Dolinsky
*
*	 Permission is hereby granted, free of charge, to any person obtaining a copy
*	 of this software and associated documentation files (the "Software"), to deal
*	 in the Software without restriction, including without limitation the rights
*	 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	 copies of the Software, and to permit persons to whom the Software is
*	 furnished to do so, subject to the following conditions:
*
*	 The above copyright notice and this permission notice shall be included in all
*	 copies or substantial portions of the Software.
*
*	 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*	 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	 SOFTWARE.
*/

#include "PhantomGunsDemo.h"
#include "TclComponent.h"


void* UTclComponent::handle = nullptr;
_Tcl_CreateInterpProto UTclComponent::_Tcl_CreateInterp = nullptr;
_Tcl_DeleteInterpProto UTclComponent::_Tcl_DeleteInterp = nullptr;
_Tcl_EvalProto UTclComponent::_Tcl_Eval = nullptr;
_Tcl_CreateObjCommandProto UTclComponent::_Tcl_CreateObjCommand = nullptr;
_Tcl_SetObjResultProto UTclComponent::_Tcl_SetObjResult = nullptr;
_Tcl_NewObjProto UTclComponent::_Tcl_NewObj = nullptr;
_Tcl_IncrRefCountProto UTclComponent::_Tcl_IncrRefCount = nullptr;
_Tcl_DecrRefCountProto UTclComponent::_Tcl_DecrRefCount = nullptr;
_Tcl_NewBooleanObjProto UTclComponent::_Tcl_NewBooleanObj = nullptr;
_Tcl_NewLongObjProto UTclComponent::_Tcl_NewLongObj = nullptr;
_Tcl_NewDoubleObjProto UTclComponent::_Tcl_NewDoubleObj = nullptr;
_Tcl_NewStringObjProto UTclComponent::_Tcl_NewStringObj = nullptr;
_Tcl_NewListObjProto UTclComponent::_Tcl_NewListObj = nullptr;
_Tcl_SetVar2ExProto UTclComponent::_Tcl_SetVar2Ex = nullptr;
_Tcl_GetVar2ExProto UTclComponent::_Tcl_GetVar2Ex = nullptr;
_Tcl_GetBooleanFromObjProto UTclComponent::_Tcl_GetBooleanFromObj = nullptr;
_Tcl_GetLongFromObjProto UTclComponent::_Tcl_GetLongFromObj = nullptr;
_Tcl_GetDoubleFromObjProto UTclComponent::_Tcl_GetDoubleFromObj = nullptr;
_Tcl_GetStringFromObjProto UTclComponent::_Tcl_GetStringFromObj = nullptr;

UTclComponent::UTclComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = false;

}

void UTclComponent::Fill(Tcl_Obj* obj) {
	if(buffer != nullptr) { buffer->refCount--; }
	buffer = obj;
	if(buffer != nullptr) { obj->refCount++; }

}
Tcl_Obj* UTclComponent::Purge() {
	auto obj = buffer;
	buffer = nullptr;
	if(obj == nullptr) { obj = _Tcl_NewObj(); } else { obj->refCount--; }
	return obj;

}
int UTclComponent::init() {
	static const Tcl_ObjType type = { "NIL", &Tcl_FreeInternalRepProc, &Tcl_DupInternalRepProc, &Tcl_UpdateStringProc, &Tcl_SetFromAnyProc };
	interpreter = _Tcl_CreateInterp();
	auto val = _Tcl_NewObj();
	val->typePtr = &type;
	*val = *(_Tcl_SetVar2Ex(interpreter, "NIL", nullptr, val, TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG));

	this->bindmethod<UTclComponent, AActor*>(this, &UTclComponent::GetOwner, "GetOwner");
	this->define("Interpreter", this);
	this->bindstatic(&UTclUnrealEssentials::PrintString, "puts");
	this->bindconvertsafe<UObject, bool, bool>(&UObject::IsValidLowLevelFast, "IsValid");

	this->bindstatic(&UTclUnrealEssentials::FindClass, "FindClass");
	this->bindstatic(&UTclUnrealEssentials::AllActorsOf, "AllActorsOf");

	this->bindconvert(static_cast<UActorComponent*(AActor::*)(const TSubclassOf<UActorComponent>) const>(&AActor::FindComponentByClass), "FindComponentByClass");

	this->bindstatic(&UTclUnrealEssentials::LineTraceSingleByChannel, "LineTraceSingleByChannel");
	this->bindstatic(&UTclUnrealEssentials::SweepSingleByChannel, "SweepSingleByChannel");
	this->bindconvert(&AActor::GetActorLocation, "GetActorLocation");
	this->bindstatic(&UTclUnrealEssentials::FindComponentsOfByTag, "FindComponentsOfByTag");
	this->bindstatic(&UTclUnrealEssentials::TypeOf, "TypeOf");
	this->bindstatic(&UTclUnrealEssentials::Eval, "Eval");
	this->bindstatic(&UTclUnrealEssentials::Purge, "Purge");
	this->bindstatic(&UTclUnrealEssentials::MAKE<FVector, float, float, float>::CONCRETE, "MakeVector");
	this->bindstatic(&UTclUnrealEssentials::MAKE<FRotator, float, float, float>::CONCRETE, "MakeRotator");
	this->bindstatic(&UTclUnrealEssentials::MAKE<FQuat, FVector, float>::CONCRETE, "MakeQuat");
	this->bindstatic(&UTclUnrealEssentials::MAKE<FLinearColor, float, float, float, float>::CONCRETE, "MakeColor");
	this->bindstatic(&UTclUnrealEssentials::ADD<FVector, FVector, FVector>::CONCRETE, "AddVectors");
	this->bindstatic(&UTclUnrealEssentials::SUB<FVector, FVector, FVector>::CONCRETE, "SubstractVectors");
	this->bindstatic(&UTclUnrealEssentials::MUL<FVector, FVector, float>::CONCRETE, "MultiplyVectorByScalar");
	this->bindstatic(&UTclUnrealEssentials::MUL<FVector, FVector, FVector>::CONCRETE, "MultiplyVectorByVector");
	this->bindstatic(&UTclUnrealEssentials::DIV<FVector, FVector, float>::CONCRETE, "DivideVectorByScalar");
	this->bindstatic(&UTclUnrealEssentials::BRACKETS<float, FVector>::CONCRETE, "GetVectorComponentByIndex");
	
	this->bindstatic(&UTclUnrealEssentials::STAR<UClass*, TSubclassOf<UObject>>::CONCRETE, "RestrictorToClass");
	
	this->bindconvert(static_cast<bool(UObject::*)(const UClass*) const>(&UObjectBaseUtility::IsA), "IsA");
	this->bindconvert(static_cast<FString(UObject::*)() const>(&UObjectBaseUtility::GetName), "GetName");
	this->bindstatic(&UTclUnrealEssentials::GENERAL_CONVERTER<UObject*>::CONCRETE, "Convert");

	this->bindstatic(&UTclUnrealEssentials::GENERAL_ACCESSOR<float>::CONCRETE, "AccessFloat");
	this->bindstatic(&UTclUnrealEssentials::GENERAL_MUTATOR<float>::CONCRETE, "MutateFloat");

	this->bindmethod<UTclComponent, void, Tcl_Obj*>(this, &UTclComponent::Fill, "Fill");
	this->bindmethod<UTclComponent, Tcl_Obj*>(this, &UTclComponent::Purge, "PurgeSelf");
	this->bindmethod<UTclComponent, UWorld*>(this, &UTclComponent::GetWorld, "GetWorld");
	this->bindstatic(&UGameplayStatics::GetPlayerController, "GetPlayerController");
	this->bindstatic(&UKismetSystemLibrary::DrawDebugLine, "DrawDebugLine");
	this->bindstatic(&UKismetSystemLibrary::DrawDebugSphere, "DrawDebugSphere");
	this->bindstatic(&UKismetMathLibrary::RandomInteger, "RandomInteger");
	this->bindstatic(&FMath::FRandRange, "RandomRange");
	this->bindstatic(&UKismetMathLibrary::Sin, "Sin");
	this->bindstatic(&UKismetMathLibrary::Asin, "Asin");
	this->bindstatic(&UKismetMathLibrary::Cos, "Cos");
	this->bindstatic(&UKismetMathLibrary::Acos, "Acos");
	this->bindstatic(&UKismetMathLibrary::RadiansToDegrees, "RadiansToDegrees");
	this->bindstatic(&UKismetMathLibrary::DegreesToRadians, "DegreesToRadians");
	this->bindstatic(&UKismetMathLibrary::FindLookAtRotation, "FindLookAtRotation");
	this->bindstatic(&UKismetMathLibrary::FTrunc, "Trunc");
	this->bindstatic(&FPlatformMath::TruncToInt, "TruncToInt");
	this->bindconvert(&USceneComponent::GetComponentLocation, "GetComponentLocation");

	this->bindflatconvert(&FVector::ToString, "VectorToString");
	this->bindflatconvert(&FRotator::ToString, "RotatorToString");

	this->bindflatconvert(&FVector::GetSafeNormal, "GetSafeNormal");
	this->bindstatic(&FVector::DotProduct, "DotProduct");
	this->bindstatic(&FVector::CrossProduct, "CrossProduct");
	this->bindconvert(&AActor::GetDistanceTo, "GetDistanceTo");
	this->bindflatconvert(&FVector::Size, "VectorMagnitude");

	this->bindconvert(&AActor::GetActorForwardVector, "GetActorForwardVector");
	this->bindconvert(&AActor::GetActorRightVector, "GetActorRightVector");
	this->bindconvert(&AActor::GetActorUpVector, "GetActorUpVector");

	this->bindconvert(&USceneComponent::GetForwardVector, "GetForwardVector");
	this->bindconvert(&USceneComponent::GetRightVector, "GetRightVector");
	this->bindconvert(&USceneComponent::GetUpVector, "GetUpVector");
	
	this->bindflatconvert(&FVector::Equals, "VectorEquals");
	this->bindstatic(&FVector::Dist, "Dist");
	this->bindflatconvert(&FVector::Rotation, "VectorToRotator");
	this->bindflatconvert(&FRotator::Equals, "RotatorEquals");
	this->bindflatconvert(&FVector::IsZero, "VectorIsZero");

	this->bindstatic(&FCollisionShape::MakeSphere, "MakeSphere");

	this->bindconvert(&UPawnMovementComponent::RequestDirectMove, "RequestDirectMove");
	this->bindconvert(&UPrimitiveComponent::SetPhysicsLinearVelocity, "SetPhysicsLinearVelocity");
	this->bindconvert(&UPrimitiveComponent::GetPhysicsLinearVelocity, "GetPhysicsLinearVelocity");
	this->bindconvert(&UPrimitiveComponent::SetPhysicsAngularVelocity, "SetPhysicsAngularVelocity");
	this->bindconvert(&UPrimitiveComponent::GetPhysicsAngularVelocity, "GetPhysicsAngularVelocity");
	this->bindconvert(&UPrimitiveComponent::AddTorque, "AddTorque");

	this->bindstatic(&FApp::GetFixedDeltaTime, "GetFixedDeltaTime");
	this->bindstatic(&FApp::GetDeltaTime, "GetDeltaTime");

	for (auto initializer : Initializers) { initializer.ExecuteIfBound(this); }
	Initializers.Empty();
	for (auto path : IncludeScriptPaths) {
		if (!path.IsEmpty()) { Eval(path, ""); }
	}
	OnBeginPlay.Broadcast();
	if (!MainScriptPath.IsEmpty()) { Eval(MainScriptPath, ""); }

	return TCL_OK;

}

void UTclComponent::BeginPlay() {
	Super::BeginPlay();
	if (handleIsMissing()) {
		auto dllPath = FPaths::Combine(*FPaths::GameDir(), TEXT("ThirdParty/"), TEXT(_TCL_DLL_FNAME_));
		if (FPaths::FileExists(dllPath)) {
			handle = FPlatformProcess::GetDllHandle(*dllPath);
			if (handle == nullptr) { UE_LOG(LogClass, Error, TEXT("Tcl bootstrapping failed")) }
			else {
				FString procName = "";
				procName = "Tcl_CreateInterp";
				_Tcl_CreateInterp = static_cast<_Tcl_CreateInterpProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_DeleteInterp";
				_Tcl_DeleteInterp = static_cast<_Tcl_DeleteInterpProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_Eval";
				_Tcl_Eval = static_cast<_Tcl_EvalProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_CreateObjCommand";
				_Tcl_CreateObjCommand = static_cast<_Tcl_CreateObjCommandProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_SetObjResult";
				_Tcl_SetObjResult = static_cast<_Tcl_SetObjResultProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_NewObj";
				_Tcl_NewObj = static_cast<_Tcl_NewObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_DbIncrRefCount";
				_Tcl_IncrRefCount = static_cast<_Tcl_IncrRefCountProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_DbDecrRefCount";
				_Tcl_DecrRefCount = static_cast<_Tcl_DecrRefCountProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_NewBooleanObj";
				_Tcl_NewBooleanObj = static_cast<_Tcl_NewBooleanObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_NewLongObj";
				_Tcl_NewLongObj = static_cast<_Tcl_NewLongObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_NewDoubleObj";
				_Tcl_NewDoubleObj = static_cast<_Tcl_NewDoubleObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_NewStringObj";
				_Tcl_NewStringObj = static_cast<_Tcl_NewStringObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_NewListObj";
				_Tcl_NewListObj = static_cast<_Tcl_NewListObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_SetVar2Ex";
				_Tcl_SetVar2Ex = static_cast<_Tcl_SetVar2ExProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_GetVar2Ex";
				_Tcl_GetVar2Ex = static_cast<_Tcl_GetVar2ExProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_GetBooleanFromObj";
				_Tcl_GetBooleanFromObj = static_cast<_Tcl_GetBooleanFromObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_GetLongFromObj";
				_Tcl_GetLongFromObj = static_cast<_Tcl_GetLongFromObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_GetDoubleFromObj";
				_Tcl_GetDoubleFromObj = static_cast<_Tcl_GetDoubleFromObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				procName = "Tcl_GetStringFromObj";
				_Tcl_GetStringFromObj = static_cast<_Tcl_GetStringFromObjProto>(FPlatformProcess::GetDllExport(handle, *procName));
				if (_Tcl_CreateInterp == nullptr ||
					_Tcl_DeleteInterp == nullptr ||
					_Tcl_Eval == nullptr ||
					_Tcl_CreateObjCommand == nullptr ||
					_Tcl_SetObjResult == nullptr ||
					_Tcl_NewObj == nullptr ||
					_Tcl_IncrRefCount == nullptr ||
					_Tcl_DecrRefCount == nullptr ||
					_Tcl_NewBooleanObj == nullptr ||
					_Tcl_NewLongObj == nullptr ||
					_Tcl_NewDoubleObj == nullptr ||
					_Tcl_NewStringObj == nullptr ||
					_Tcl_NewListObj == nullptr ||
					_Tcl_SetVar2Ex == nullptr ||
					_Tcl_GetVar2Ex == nullptr ||
					_Tcl_GetBooleanFromObj == nullptr ||
					_Tcl_GetLongFromObj == nullptr ||
					_Tcl_GetDoubleFromObj == nullptr ||
					_Tcl_GetStringFromObj == nullptr) {
					handle = nullptr;
					UE_LOG(LogClass, Error, TEXT("Bootstrapping one or more functions for Tcl failed!"))
				}
				else {
					init();
					UE_LOG(LogClass, Log, TEXT("Bootstrapping Tcl and its functions succeeded!"))
				}
			}
		}
		else { UE_LOG(LogClass, Error, TEXT("Cannot find %s for Tcl!"), _TCL_DLL_FNAME_) }
	} else { init(); }
	
}

void UTclComponent::BeginDestroy() {
	Super::BeginDestroy();
	if(interpreter != nullptr) { _Tcl_DeleteInterp(interpreter); }

}

void UTclComponent::Tcl_FreeInternalRepProc(Tcl_Obj* obj) { }
void UTclComponent::Tcl_DupInternalRepProc(Tcl_Obj* srcPtr, Tcl_Obj* dupPtr) { }
void UTclComponent::Tcl_UpdateStringProc(Tcl_Obj* obj) { }
int UTclComponent::Tcl_SetFromAnyProc(Tcl_Interp* interp, Tcl_Obj* obj) { return 0; }

bool UTclComponent::handleIsMissing() { return handle == nullptr; }

_Tcl_CreateObjCommandProto UTclComponent::get_Tcl_CreateObjCommand() { return _Tcl_CreateObjCommand; }
_Tcl_SetObjResultProto UTclComponent::get_Tcl_SetObjResult() { return _Tcl_SetObjResult; }
_Tcl_SetVar2ExProto UTclComponent::get_Tcl_SetVar2Ex() { return _Tcl_SetVar2Ex; }
_Tcl_NewObjProto UTclComponent::get_Tcl_NewObj() { return _Tcl_NewObj; }
_Tcl_NewBooleanObjProto UTclComponent::get_Tcl_NewBooleanObj() { return _Tcl_NewBooleanObj; }
_Tcl_NewLongObjProto UTclComponent::get_Tcl_NewLongObj() { return _Tcl_NewLongObj; };
_Tcl_NewDoubleObjProto UTclComponent::get_Tcl_NewDoubleObj() { return _Tcl_NewDoubleObj; }
_Tcl_NewStringObjProto UTclComponent::get_Tcl_NewStringObj() { return _Tcl_NewStringObj; }
_Tcl_NewListObjProto UTclComponent::get_Tcl_NewListObj() { return _Tcl_NewListObj; }
_Tcl_GetBooleanFromObjProto UTclComponent::get_Tcl_GetBooleanFromObj() { return _Tcl_GetBooleanFromObj; }
_Tcl_GetLongFromObjProto UTclComponent::get_Tcl_GetLongFromObj() { return _Tcl_GetLongFromObj; }
_Tcl_GetDoubleFromObjProto UTclComponent::get_Tcl_GetDoubleFromObj() { return _Tcl_GetDoubleFromObj; }
_Tcl_GetStringFromObjProto UTclComponent::get_Tcl_GetStringFromObj() { return _Tcl_GetStringFromObj; }

int UTclComponent::eval(const char* code) {
	if(handleIsMissing() || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; } else { return _Tcl_Eval(interpreter, code); }

}

int32 UTclComponent::Eval(FString Filename, FString Code) {
	FString fname = "";
	if(!Filename.IsEmpty()) {
		fname = FPaths::GameContentDir() + "Scripts/" + Filename;
		if(FPaths::FileExists(fname)) {
			FFileHelper::LoadFileToString(Code, *fname);
		} else {
			UE_LOG(LogClass, Warning, TEXT("File at path: %s doesn't exist! Executing the Code field"), *fname)
		}
	}
	auto status = eval(TCHAR_TO_ANSI(*Code));
	if (status == TCL_ERROR) { UE_LOG(LogClass, Error, TEXT("Tcl script error for! filepath: '%s' with code: '%s'!"), *fname, *Code) }
	return status;

}

int32 UTclComponent::SetObj(UObject* Object, FString Location, FString Key) { return this->define(Location, Object, Key); }
int32 UTclComponent::SetClass(TSubclassOf<UObject> Class, FString Location, FString Key) { return this->define(Location, Class, Key); }

int32 UTclComponent::GetFloat(FString Location, FString Key, float& Result) {
	if (handle == nullptr || interpreter == nullptr) { return _TCL_BOOTSTRAP_FAIL_; } else {
		if (Location.IsEmpty()) {
			UE_LOG(LogClass, Warning, TEXT("Tcl: Location parameter cannot be empty for the GetFloat function!"))
			return TCL_ERROR;
		} else {
			double in = 0.f;
			auto obj = _Tcl_GetVar2Ex(interpreter, TCHAR_TO_ANSI(*Location), Key.IsEmpty() ? nullptr : TCHAR_TO_ANSI(*Key), TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
			auto result = _Tcl_GetDoubleFromObj(interpreter, obj, &in);
			Result = in;
			return result;
		}
	}

}
UObject* UTclComponent::GetObj(FString Location, FString Key) {
	static const FString genericType = "UObject";
	UObject* Result = nullptr;
	if (!(handle == nullptr || interpreter == nullptr)) {
		if (Location.IsEmpty()) { UE_LOG(LogClass, Warning, TEXT("Tcl: Location parameter cannot be empty for the GetObj function!")) } else {
			auto obj = _Tcl_GetVar2Ex(interpreter, TCHAR_TO_ANSI(*Location), Key.IsEmpty() ? nullptr : TCHAR_TO_ANSI(*Key), TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
			if (obj == nullptr || obj->typePtr == nullptr) { UE_LOG(LogClass, Error, TEXT("Tcl error! Failed to get an object.")) } else {
				FString name = obj->typePtr->name;
				if (name == genericType) { Result = static_cast<UObject*>(obj->internalRep.otherValuePtr); }
			}
		}
	}
	return Result;

}