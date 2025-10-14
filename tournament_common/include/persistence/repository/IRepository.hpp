#ifndef RESTAPI_IREPOSITORY_HPP
#define RESTAPI_IREPOSITORY_HPP

#include <vector>
#include <memory>
#include <string>
#include <optional> // CAMBIO: Incluir para std::optional

template<class T, class Y>
class IRepository {
public:
    virtual ~IRepository() = default;
    
    // CAMBIO: La firma ahora devuelve std::optional<std::string>
    virtual std::optional<std::string> Create(const T& entity) = 0;
    
    virtual std::shared_ptr<T> ReadById(Y id) = 0;
    virtual std::vector<std::shared_ptr<T>> ReadAll() = 0;
    
    // CAMBIO: La firma ahora usa std::string
    virtual std::string Update(const T& entity) = 0;
    virtual void Delete(Y id) = 0;
};

#endif //RESTAPI_IREPOSITORY_HPP