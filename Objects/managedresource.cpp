#include "managedresource.h"

ManagedResource::ManagedResource(QString name, int resourceId, int count)
{
    this->name = name;
    this->count = count;
    this->resourceId = resourceId;
}
