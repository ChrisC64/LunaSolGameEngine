//module;
//#include "LSEFramework.h"
//export module Data.LSBufferTypes;
//
//export namespace LS
//{
//
//	/***
//	 * @enum BUFFER_USAGE
//	 * @brief Details how the buffer will be used which effects the GPU/CPU read/write access.
//	 */
//	enum class BUFFER_USAGE
//	{
//		DEFAULT_RW, // @brief Buffer can be read and written by the GPU. 
//		CONSTANT,// @brief Buffer is meant to be used by shader's constant buffers
//		IMMUTABLE, // @brief Buffer lives on GPU but can never be changed after initialization, and remains the same
//		DYNAMIC, // @brief Buffer can be modified by CPU and read by the GPU only. 
//		COPY_ONLY // @brief Buffer can copy contents from GPU to CPU
//	};
//
//	/***
//	 * @enum BUFFER_PIPELINE_STAGES
//	 * @brief The description of what pipeline stage this buffer object will be used for in the graphics pipeline.
//	 */
//	enum class BUFFER_PIPELINE_STAGES
//	{
//		UNDEFINED,
//		VERTEX, // @brief Buffer can be used for vertex pipeline
//		INDEX, // @brief Buffer can be used for index pipeline
//		HULL, // @brief Buffer is used for Hull-shader pipeline
//		TESSELATOR, // @brief Buffer is used for Tesselator pipeline
//		GEOMETRY, // @brief Buffer is used for Geometry pipeline
//		CONSTANT, // @brief Buffer is constant buffer data used by shader objects(not a pipeline stage)
//		SHADER_RESOURCE // @brief Buffer is a resource (texture or other type) that is used for shaders (not a constant buffer data)
//	};
//	//TODO: DO I still want this...?
//	/**
//	 * @brief A helper struct for informing how to create a LS::Buffer
//	 */
//	struct LSBufferInfo
//	{
//		BUFFER_USAGE Usage{}; // @brief Usage of buffer (vertex, index, etc.)
//		BUFFER_PIPELINE_STAGES Stage{}; //@brief Role in the Graphics Pipeline Stage
//		std::byte* pData = nullptr;
//		size_t Stride = 0; //@brief The size of the struct if this buffer contains more than 1 (otherwise leave 0)
//		size_t Count = 0; //@brief Number of strides (otherwise leave 0 if only using 1)
//		size_t Size = 0; //@brief Total size in bytes
//	};
//}