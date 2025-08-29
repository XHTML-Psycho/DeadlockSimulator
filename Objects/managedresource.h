#ifndef MANAGEDRESOURCE_H
#define MANAGEDRESOURCE_H

#include <QString>

/**
 * @brief Class which represents the resources used by processes
 */
class ManagedResource
{

private:
    /**
     * @brief name of a resource
     * @brief resourceId corresponds to a resource: (0 = Printer, 1 = CD-ROM, 2 = Plotter, 3 = TapeDrive)
     * @brief count of the resource (how many are physically available)
     */
    QString name;
    int resourceId;
    int count;

public:
    /**
     * @brief ManagedResource constructor
     * @param name of a resource
     * @param resourceId corresponds to a resource: (0 = Printer, 1 = CD-ROM, 2 = Plotter, 3 = TapeDrive)
     * @param count of the resource (how many are physically available)
     */
    ManagedResource(QString name, int resourceId, int count);

    /**
     * @brief getResourceId getter for resourceId
     * @return resourceId
     */
    int getResourceId() const{
        return resourceId;
    }

    /**
     * @brief setResourceId setter for resourceId
     * @param resourceId
     */
    void setResourceId(int resourceId);

    /**
     * @brief getName getter for name
     * @return name
     */
    const QString &getName() const {
        return name;
    }

    /**
     * @brief setName setter for name
     * @param name
     */
    void setName(const QString &name) {
        ManagedResource::name = name;
    }

    /**
     * @brief getCount getter for count
     * @return count
     */
    int getCount() const {
        return count;
    }

    /**
     * @brief setCount setter for count
     * @param count
     */
    void setCount(int count) {
        this->count = count;
    }

    /**
     * @brief decreaseCount decreases the count
     * @param count
     */
    void decreaseCount(int count){
        this->count -= count;
    }

    bool operator==(const ManagedResource& otherResource) const{
        return resourceId == otherResource.resourceId and name == otherResource.name;
    }
};

#endif // MANAGEDRESOURCE_H
