/*
    Environment for Eva interpreter
*/

class Environment {

    /*
        Constructor for record
    */
    constructor(record = {}, parent = null) {
        this.record = record;
        this.parent = parent;
    }

    /*
        Create variable with name and value
    */
    define(name, value) {
        this.record[name] = value;
        return value;
    }

    /*
        Update existing variable
    */
    assign(name, value) {
        this.resolve(name).record[name] = value;
        return value;
    }

    /*
        Return value of defined variable
    */
    lookup(name) {
        // if (!this.record.hasOwnProperty(name)) {
        //     throw new ReferenceError(`variable "${name}" is not defined`);
        // }
        // return this.record[name];
        return this.resolve(name).record[name];
    }

    /*
        Identifier resolution - return environment where variable is defined (scope chain traversal)
    */
    resolve(name) {
        if (this.record.hasOwnProperty(name)) {
            return this;
        }

        if (this.parent == null) {
            throw new ReferenceError(`variable "${name}" is not defined`);
        }

        return this.parent.resolve(name);
    }
}

module.exports = Environment;
