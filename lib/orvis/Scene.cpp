#include "Scene.hpp"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

#include <numeric>
#include <execution>
#include <vector>

#include "Util.hpp"
#include "stb/stb_image.h"
#include <iostream>

Scene::Scene(const std::filesystem::path& filename)
{
	const auto path = util::resourcesPath / filename;
	const auto pathString = path.string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::cout << "Loading model from " << filename.string() << std::endl;

	std::string err;
	const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, pathString.c_str());

	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret || shapes.empty()) {
		throw std::runtime_error("OBJ import failed!");
	}

	std::cout << "OBJ import complete. Processing Model..." << std::endl;

	std::vector<glm::vec4> allVertices(attrib.vertices.size() / 3);

#pragma omp parallel for
	for (int i = 0; i < attrib.vertices.size(); i += 3)
	{
		allVertices[i / 3] = glm::vec4(attrib.vertices[i + 0], attrib.vertices[i + 1], attrib.vertices[i + 2], 1.0f);
	}

	std::vector<glm::vec4> allNormals(attrib.normals.size() / 3);

#pragma omp parallel for
	for (int i = 0; i < attrib.normals.size(); i += 3)
	{
		allNormals[i / 3] = glm::vec4(attrib.normals[i + 0], attrib.normals[i + 1], attrib.normals[i + 2], 0.0f);
	}

	std::vector<glm::vec2> allUVs(attrib.texcoords.size() / 2);

#pragma omp parallel for
	for (int i = 0; i < attrib.texcoords.size(); i += 2)
	{
		allUVs[i / 2] = glm::vec2(attrib.texcoords[i + 0], attrib.texcoords[i + 1]);
	}

	std::vector<GLuint> allIndices;
	std::vector<IndirectDrawCommand> indirectDrawParams;
	GLuint start = 0;

	for (auto& s : shapes)
	{
		Bounds b;
		for (auto& i : s.mesh.indices)
		{
			allIndices.push_back(i.vertex_index);

			b = b + glm::vec3(allVertices[i.vertex_index]);			
		}

		const auto count = static_cast<GLuint>(s.mesh.indices.size());

		m_meshes.push_back(std::shared_ptr<Mesh>(new Mesh(start, start + count/*, materials[s.mesh.material_ids[0]]*/)));

		indirectDrawParams.push_back({ count, 1U, start, 0U, 0U });

		start += count;

		m_meshes[m_meshes.size() - 1]->setBoundingBox(b);
	}

	m_multiDrawIndexBuffer.resize(allIndices.size(), GL_DYNAMIC_STORAGE_BIT);
	m_multiDrawIndexBuffer.assign(allIndices);
	m_multiDrawVertexBuffer.resize(attrib.vertices.size(), GL_DYNAMIC_STORAGE_BIT);
	m_multiDrawVertexBuffer.assign(allVertices);
	m_multiDrawNormalBuffer.resize(allNormals.size(), GL_DYNAMIC_STORAGE_BIT);
	m_multiDrawNormalBuffer.assign(allNormals);
	m_multiDrawUVBuffer.resize(allUVs.size(), GL_DYNAMIC_STORAGE_BIT);
	m_multiDrawUVBuffer.assign(allUVs);
	m_indirectDrawBuffer.resize(indirectDrawParams.size(), GL_DYNAMIC_STORAGE_BIT);
	m_indirectDrawBuffer.assign(indirectDrawParams);

	m_multiDrawVao.format(VertexAttributeBinding::vertices, 4, GL_FLOAT, false, 0);
	m_multiDrawVao.setVertexBuffer(m_multiDrawVertexBuffer, VertexAttributeBinding::vertices, 0, sizeof(glm::vec4));
	m_multiDrawVao.binding(VertexAttributeBinding::vertices);

	m_multiDrawVao.format(VertexAttributeBinding::normals, 4, GL_FLOAT, true, 0);
	m_multiDrawVao.setVertexBuffer(m_multiDrawNormalBuffer, VertexAttributeBinding::normals, 0, sizeof(glm::vec4));
	m_multiDrawVao.binding(VertexAttributeBinding::normals);

	m_multiDrawVao.format(VertexAttributeBinding::texCoords, 2, GL_FLOAT, false, 0);
	m_multiDrawVao.setVertexBuffer(m_multiDrawUVBuffer, VertexAttributeBinding::texCoords, 0, sizeof(glm::vec2));
	m_multiDrawVao.binding(VertexAttributeBinding::texCoords);

	m_multiDrawVao.setElementBuffer(m_multiDrawIndexBuffer);

	//// - - - M E S H E S - - -
	//const auto numMeshes = shapes.size();
	//m_meshes.resize(numMeshes);
	//for (int i = 0; i < static_cast<int>(numMeshes); ++i)
	//{
	//	m_meshes[i] = std::shared_ptr<Mesh>(new Mesh(shapes[i].mesh, materials[shapes[i].mesh.material_ids[0]], attrib, path));
	//}

	std::thread bBoxThread([&]()
	{
		calculateBoundingBox();
	});

	//{ // running in parallel --> no bounding boxes available in this scope!
	//	updateMultiDrawBuffers();

		updateMaterialBuffer();

		m_cullingProgram.attachNew(GL_COMPUTE_SHADER, ShaderFile::load("compute/viewFrustumCulling.comp"));

		m_lightIndexBuffer.resize(1, GL_DYNAMIC_STORAGE_BIT);
	//}

	bBoxThread.join();

	updateModelMatrices();
	updateBoundingBoxBuffer();

	std::cout << "Loading complete: " << filename.string() << std::endl;
}

void Scene::render(const Program& program, bool overwriteCameraBuffer) const
{
	// BINDINGS
	m_indirectDrawBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::indirectDraw);
	m_bBoxBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::boundingBoxes);
	m_modelMatBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::modelMatrices);
	m_materialBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::materials);
	m_lightBuffer.bind(GL_SHADER_STORAGE_BUFFER, BufferBinding::lights);
	if (overwriteCameraBuffer)
		m_camera->uploadToGpu();

	// CULLING
	m_cullingProgram.use();
	glDispatchCompute(static_cast<GLuint>(glm::ceil(m_indirectDrawBuffer.size() / 64.0f)), 1, 1);
	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

	// DRAW
	program.use();
	m_multiDrawVao.bind();
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, *m_indirectDrawBuffer.id());
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(m_indirectDrawBuffer.size()), 0);
}

const Bounds& Scene::calculateBoundingBox()
{
	struct Reduction
	{
		Bounds operator()(Bounds b, const std::shared_ptr<Mesh>& x) const {
			return b +
				Bounds(x->modelMatrix * glm::vec4(x->bounds.min, 1.0f), x->modelMatrix * glm::vec4(x->bounds.max, 1.0f));
		}

		Bounds operator()(const std::shared_ptr<Mesh>& x, Bounds b) const {
			return b +
				Bounds(x->modelMatrix * glm::vec4(x->bounds.min, 1.0f), x->modelMatrix * glm::vec4(x->bounds.max, 1.0f));
		}

		Bounds operator()(const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) const
		{
			Bounds bounds;
			bounds = bounds + Bounds(a->modelMatrix * glm::vec4(a->bounds.min, 1.0f), a->modelMatrix * glm::vec4(a->bounds.max, 1.0f));
			return bounds + Bounds(b->modelMatrix * glm::vec4(b->bounds.min, 1.0f), b->modelMatrix * glm::vec4(b->bounds.max, 1.0f));
		}

		Bounds operator()(Bounds b, const Bounds& x) const { return b + x; }
	};

	bounds = std::reduce(std::execution::par_unseq, m_meshes.begin(), m_meshes.end(), Bounds(), Reduction());
	if (m_camera)
		m_camera->setSpeed(0.1f * glm::length(bounds[1] - bounds[0]));
	return bounds;
}

void Scene::updateModelMatrices()
{
	std::vector<glm::mat4> modelMatrices(m_meshes.size());

#pragma omp parallel for
	for (int i = 0; i < static_cast<int>(modelMatrices.size()); ++i)
		modelMatrices[i] = m_meshes[i]->modelMatrix;

	if (modelMatrices.size() != static_cast<size_t>(m_modelMatBuffer.size()))
		m_modelMatBuffer.resize(modelMatrices.size(), GL_DYNAMIC_STORAGE_BIT);

	m_modelMatBuffer.assign(modelMatrices);
}

void Scene::updateBoundingBoxBuffer()
{
	std::vector<Bounds> boundingBoxes(m_meshes.size());

#pragma omp parallel for
	for (int i = 0; i < static_cast<int>(boundingBoxes.size()); ++i)
		boundingBoxes[i] = m_meshes[i]->bounds;

	if (boundingBoxes.size() != static_cast<size_t>(m_bBoxBuffer.size()))
		m_bBoxBuffer.resize(boundingBoxes.size(), GL_DYNAMIC_STORAGE_BIT);

	m_bBoxBuffer.assign(boundingBoxes);
}

void Scene::updateMaterialBuffer()
{
	std::vector<Material> materials(m_meshes.size());

#pragma omp parallel for
	for (int i = 0; i < static_cast<int>(materials.size()); ++i)
		materials[i] = m_meshes[i]->material;

	if (materials.size() != static_cast<size_t>(m_materialBuffer.size()))
		m_materialBuffer.resize(materials.size(), GL_DYNAMIC_STORAGE_BIT);

	m_materialBuffer.assign(materials);
}

void Scene::updateLightBuffer()
{
	std::vector<Light> lights(m_lights.size());

#pragma omp parallel for
	for (int i = 0; i < static_cast<int>(lights.size()); ++i)
	{
		m_lights[i]->recalculateLightSpaceMatrix(*this);
		lights[i] = *m_lights[i];
	}

	if (lights.size() != static_cast<size_t>(m_lightBuffer.size()))
		m_lightBuffer.resize(lights.size(), GL_DYNAMIC_STORAGE_BIT);

	m_lightBuffer.assign(lights);
}

void Scene::updateShadowMaps()
{
	for (int i = 0; i < static_cast<int>(m_lights.size()); ++i)
	{
		m_lightIndexBuffer.assign(i);
		m_lightIndexBuffer.bind(GL_UNIFORM_BUFFER, BufferBinding::lightIndex);
		m_lights[i]->updateShadowMap(*this);
	}
}

void Scene::addMesh(const std::shared_ptr<Mesh>& mesh)
{
	//m_meshes.push_back(mesh);
	//std::thread bBoxThread([&]() { calculateBoundingBox(); });
	//updateModelMatrices();
	//updateMultiDrawBuffers();
	//updateBoundingBoxBuffer();
	//updateMaterialBuffer();
	//bBoxThread.join();
}

void Scene::addLight(const std::shared_ptr<Light>& light)
{
	m_lights.push_back(light);
	updateLightBuffer();
	updateShadowMaps();
}

void Scene::setCamera(const std::shared_ptr<Camera>& camera)
{
	m_camera = camera;
	m_camera->setSpeed(0.1f * glm::length(bounds[1] - bounds[0]));
}

std::shared_ptr<Camera> Scene::getCamera() const
{
	return m_camera;
}

const std::deque<std::shared_ptr<Mesh>>& Scene::getMeshes() const
{
	return m_meshes;
}

void Scene::reorderMeshes()
{
	//std::deque<std::shared_ptr<Mesh>> orderedMeshes;

	//for (const auto& mesh : m_meshes)
	//{
	//	if (mesh->isTransparent())
	//		orderedMeshes.push_back(mesh);
	//	else
	//		orderedMeshes.push_front(mesh);
	//}

	//m_meshes = orderedMeshes;

	//updateModelMatrices();
	//updateMultiDrawBuffers();
	//updateBoundingBoxBuffer();
	//updateMaterialBuffer();
}

void Scene::updateMultiDrawBuffers()
{
	//std::vector<GLuint> allIndices;
	//std::vector<glm::vec4> allVertices;
	//std::vector<glm::vec4> allNormals;
	//std::vector<glm::vec2> allUVs;

	//std::vector<IndirectDrawCommand> indirectDrawParams;

	//GLuint start = 0;
	//GLuint baseVertexOffset = 0;
	//for (const auto& mesh : m_meshes)
	//{
	//	allIndices.insert(allIndices.end(), mesh->indices.begin(), mesh->indices.end());
	//	allVertices.insert(allVertices.end(), mesh->vertices.begin(), mesh->vertices.end());
	//	allNormals.insert(allNormals.end(), mesh->normals.begin(), mesh->normals.end());
	//	allUVs.insert(allUVs.end(), mesh->uvs.begin(), mesh->uvs.end());

	//	const auto count = static_cast<GLuint>(mesh->indices.size());

	//	indirectDrawParams.push_back({ count, 1U, start, baseVertexOffset, 0U });

	//	start += count;
	//	baseVertexOffset += static_cast<GLuint>(mesh->vertices.size());
	//}

	//m_multiDrawIndexBuffer.resize(allIndices.size(), GL_DYNAMIC_STORAGE_BIT);
	//m_multiDrawIndexBuffer.assign(allIndices);
	//m_multiDrawVertexBuffer.resize(allVertices.size(), GL_DYNAMIC_STORAGE_BIT);
	//m_multiDrawVertexBuffer.assign(allVertices);
	//m_multiDrawNormalBuffer.resize(allNormals.size(), GL_DYNAMIC_STORAGE_BIT);
	//m_multiDrawNormalBuffer.assign(allNormals);
	//m_multiDrawUVBuffer.resize(allUVs.size(), GL_DYNAMIC_STORAGE_BIT);
	//m_multiDrawUVBuffer.assign(allUVs);
	//m_indirectDrawBuffer.resize(indirectDrawParams.size(), GL_DYNAMIC_STORAGE_BIT);
	//m_indirectDrawBuffer.assign(indirectDrawParams);

	//m_multiDrawVao.format(VertexAttributeBinding::vertices, 4, GL_FLOAT, false, 0);
	//m_multiDrawVao.setVertexBuffer(m_multiDrawVertexBuffer, VertexAttributeBinding::vertices, 0, sizeof(glm::vec4));
	//m_multiDrawVao.binding(VertexAttributeBinding::vertices);

	//m_multiDrawVao.format(VertexAttributeBinding::normals, 4, GL_FLOAT, true, 0);
	//m_multiDrawVao.setVertexBuffer(m_multiDrawNormalBuffer, VertexAttributeBinding::normals, 0, sizeof(glm::vec4));
	//m_multiDrawVao.binding(VertexAttributeBinding::normals);

	//m_multiDrawVao.format(VertexAttributeBinding::texCoords, 2, GL_FLOAT, false, 0);
	//m_multiDrawVao.setVertexBuffer(m_multiDrawUVBuffer, VertexAttributeBinding::texCoords, 0, sizeof(glm::vec2));
	//m_multiDrawVao.binding(VertexAttributeBinding::texCoords);

	//m_multiDrawVao.setElementBuffer(m_multiDrawIndexBuffer);
}
