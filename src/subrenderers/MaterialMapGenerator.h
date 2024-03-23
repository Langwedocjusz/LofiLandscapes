#pragma once

#include "MapGenerator.h"

class MaterialMapGenerator{
public:
    MaterialMapGenerator(ResourceManager& manager, const MapGenerator& map);
    void Init(int res, int wrap_type);

    void BindMaterialmap(int id=0) const;
   
    void OnUpdate();
    void RequestUpdate() {m_UpdateQueued = true;}
    void OnImGui(bool& open);
    
    void OnSerialize(nlohmann::ordered_json& output);
    void OnDeserialize(nlohmann::ordered_json& input);
private:
    bool m_UpdateQueued = true;

    TextureEditor m_MaterialEditor;
    std::shared_ptr<Texture2D> m_Materialmap;
   
    const MapGenerator& m_Map;
    ResourceManager& m_ResourceManager;
};
