#pragma once


/// <summary>
/// Singleton for controlling the creation of game entities.
/// </summary>
/// <author>Ben Hoffman</author>
class EntityManager
{
public:

    /// <summary>
    /// Getter for the InputManager
    /// </summary>
    /// <returns>Singleton instance of the input manager</returns>
    static EntityManager* GetInstance();

    /// <summary>
    /// Release the current singleton instance
    /// </summary>
    static void ReleaseInstance();

private:

    /// <summary>
    /// Private constructor for the entity manager
    /// </summary>
    EntityManager();

    /// <summary>
    /// Private destructor for the entity manager
    /// </summary>
    ~EntityManager();

    static EntityManager* Instance;

};
