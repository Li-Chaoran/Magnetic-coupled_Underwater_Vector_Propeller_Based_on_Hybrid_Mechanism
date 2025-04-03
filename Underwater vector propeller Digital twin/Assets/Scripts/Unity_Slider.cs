using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Unity_Slider : MonoBehaviour
{
    public GameObject chuanlianliangan1;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void OnValueChanged(float value)
    {
        chuanlianliangan1.GetComponent<joint1_rotation>().dm3410_pose = value;
        // Debug.Log("Slider value changed: " + value);
    }
}
