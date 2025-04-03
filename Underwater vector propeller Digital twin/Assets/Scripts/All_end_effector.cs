using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class All_end_effector : MonoBehaviour
{
    public float joint1 = 180.0f;
    // Start is called before the first frame update
    void Start()
    {
        // transform.SetPositionAndRotation(new Vector3(-0.2389f,-0.1485f,0.0f), Quaternion.Euler(0.0f, 0.0f, 0.0f));
        transform.SetPositionAndRotation(transform.position, Quaternion.Euler(180.0f, 0.0f, 0.0f));
    }

    // Update is called once per frame
    void Update()
    {
        int c = Input.GetKey(KeyCode.C) ? 1 : 0;
        int v = Input.GetKey(KeyCode.V) ? 1 : 0;
        if (c != 0)
        {
            joint1 = joint1 + c*0.01f; 
            transform.SetPositionAndRotation(transform.position, Quaternion.Euler(-joint1, 0.0f, 0.0f));
        };
        if (v != 0)
        {
            joint1 = joint1 - v*0.01f; 
            transform.SetPositionAndRotation(transform.position, Quaternion.Euler(-joint1, 0.0f, 0.0f));
        };
    }
}
